// assets/loaders/MeshLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"
#include "core/Paths.h"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <map>
#include <vector>

namespace RDE {
    class MeshLoader final : public ILoader {
    public:
        // No longer needs to store a pointer to the manager.
        MeshLoader() = default;

        // --- 1. Fast Dependency Discovery ---
        std::vector<std::string> get_dependencies(const std::string &uri) const override {
            std::vector<std::string> dependencies;

            // We must parse the materials to find their texture dependencies.
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;
            std::string base_dir = std::filesystem::path(uri).parent_path().string();

            // We only need the materials, not the full geometry, but LoadObj gives us all.
            tinyobj::LoadObj(nullptr, nullptr, &materials, &warn, &err, uri.c_str(), base_dir.c_str(), true);

            // A. Find all texture dependencies from the .mtl file.
            for (const auto &mtl: materials) {
                if (!mtl.diffuse_texname.empty()) {
                    // Resolve relative path and add to dependency list
                    dependencies.push_back((std::filesystem::path(base_dir) / mtl.diffuse_texname).string());
                }
                // Add other maps here (normal, metallic, roughness, etc.) if your shaders support them.
            }

            // B. The materials we generate will need a shader. This is an implicit dependency.
            // This path should match the one hardcoded in `load_asset`.
            dependencies.emplace_back("shaders/basic_lit.shaderdef"); // Assuming you use a .shaderdef now

            // C. Add the fallback default material as a dependency.
            dependencies.push_back((get_asset_path().value() / "materials" / "default.mat").string());

            return dependencies;
        }

        // --- 2. The Actual Loading Function ---
        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override {
            RDE_CORE_INFO("MeshLoader: Loading asset from '{}'...", uri);

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;
            std::string base_dir = std::filesystem::path(uri).parent_path().string();

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, uri.c_str(), base_dir.c_str(), true)) {
                RDE_CORE_ERROR("MeshLoader: Failed to load '{}'. Err: {}", uri, err);
                return nullptr;
            }
            if (!warn.empty())
                RDE_CORE_WARN("MeshLoader: Warning for '{}': {}", uri, warn);

            // --- A. Process Materials (In-Memory) ---
            // Convert each tinyobj::material_t into an engine-native material asset directly in the database.
            std::vector<AssetID> material_asset_ids;
            for (const auto &mtl: materials) {
                material_asset_ids.push_back(create_material_in_db(mtl, base_dir, db, manager, uri));
            }

            // If the model had no materials, use the default material (which is already loaded as a dependency).
            if (material_asset_ids.empty()) {
                RDE_CORE_WARN("MeshLoader: No materials found for '{}'. Using default material.", uri);
                AssetID default_mat_id = manager.get_loaded_asset(
                    (get_asset_path().value() / "materials" / "default.mat").string());
                if (default_mat_id) {
                    material_asset_ids.push_back(default_mat_id);
                } else {
                    RDE_CORE_ERROR("MeshLoader: Default material could not be found in cache for '{}'!", uri);
                    return nullptr;
                }
            }

            // --- B. Create the main Prefab Asset ---
            auto &registry = db.get_registry();
            auto prefab_entity_id = registry.create();
            registry.emplace<AssetFilepath>(prefab_entity_id, uri);
            registry.emplace<AssetName>(prefab_entity_id, std::filesystem::path(uri).stem().string());
            auto &prefab_comp = registry.emplace<AssetPrefab>(prefab_entity_id);

            // --- C. Process Geometry ---
            // This is complex, so we keep it in a helper function.
            for (const auto &shape: shapes) {
                create_submeshes_from_shape(prefab_comp, shape, attrib, materials, material_asset_ids, db, uri);
            }

            RDE_CORE_TRACE("MeshLoader: Processed '{}' into a prefab with {} sub-meshes.", uri,
                           prefab_comp.child_assets.size());
            return std::make_shared<AssetID_Data>(prefab_entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".obj"};
        }

    private:
        // --- NEW: Material Creation Helper ---
        // This function REPLACES generating a .mat file. It creates the material directly in the database.
        static AssetID create_material_in_db(const tinyobj::material_t &mtl, const std::string &obj_base_dir,
                                             AssetDatabase &db, AssetManager &manager, const std::string &parent_uri) {
            // This virtual URI uniquely identifies the material generated by this mesh.
            std::string virtual_uri = parent_uri + "#" + mtl.name;

            // Check if we've already created this material (e.g., if loader is re-run).
            if (AssetID existing_id = manager.get_loaded_asset(virtual_uri)) {
                return existing_id;
            }

            AssetCpuMaterial cpu_mat;
            cpu_mat.name = mtl.name;
            cpu_mat.shader_path = "shaders/basic_lit.shaderdef"; // Must match dependency
            cpu_mat.cull_mode = RAL::CullMode::Back;
            cpu_mat.depth_test = true;
            cpu_mat.depth_write = true;
            cpu_mat.vector_params["baseColor"] = {mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2], 1.0f};
            float roughness = 1.0f - (std::clamp(mtl.shininess, 0.f, 1000.f) / 1000.f);
            cpu_mat.float_params["roughness"] = roughness;
            cpu_mat.float_params["metalness"] = (mtl.specular[0] > 0.5f) ? 0.9f : 0.1f;

            if (!mtl.diffuse_texname.empty()) {
                std::string texture_path = (std::filesystem::path(obj_base_dir) / mtl.diffuse_texname).string();
                // Get the already-loaded texture ID from the manager
                if (AssetID texture_id = manager.get_loaded_asset(texture_path)) {
                    cpu_mat.texture_asset_ids["albedoMap"] = texture_id;
                } else {
                    RDE_CORE_ERROR("MeshLoader: Texture dependency '{}' not found in cache for material '{}'!",
                                   texture_path, mtl.name);
                }
            }

            auto &registry = db.get_registry();
            auto entity_id = registry.create();
            registry.emplace<AssetCpuMaterial>(entity_id, std::move(cpu_mat));
            registry.emplace<AssetName>(entity_id, mtl.name);
            registry.emplace<AssetFilepath>(entity_id, virtual_uri); // Store the virtual URI

            // Create and return the AssetID for this new in-memory asset.
            return std::make_shared<AssetID_Data>(entity_id, virtual_uri);
        }

        template<class T>
        static void hash_combine(std::size_t &seed, const T &v) {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // The geometry processing helper remains largely the same, but is now static.
        // Hashing helpers are also static. (Code omitted for brevity, it's identical to your original).
        static void create_submeshes_from_shape(AssetPrefab &prefab, const tinyobj::shape_t &shape,
                                                const tinyobj::attrib_t &attrib,
                                                const std::vector<tinyobj::material_t> &materials,
                                                const std::vector<AssetID> &material_asset_ids, AssetDatabase &db,
                                                const std::string &original_uri) {
            // Group faces by the material ID assigned to them.
            std::map<int, std::vector<tinyobj::index_t> > faces_by_material;
            for (size_t i = 0; i < shape.mesh.indices.size() / 3; ++i) {
                int material_id = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[i];
                faces_by_material[material_id].push_back(shape.mesh.indices[3 * i + 0]);
                faces_by_material[material_id].push_back(shape.mesh.indices[3 * i + 1]);
                faces_by_material[material_id].push_back(shape.mesh.indices[3 * i + 2]);
            }

            // Now, create a separate mesh asset for each group of faces.
            for (const auto &[material_idx, face_indices]: faces_by_material) {
                if (face_indices.empty()) {
                    continue;
                }

                auto hash_func = [](const tinyobj::index_t &i) {
                    size_t seed = 0;
                    hash_combine(seed, i.vertex_index);
                    hash_combine(seed, i.normal_index);
                    hash_combine(seed, i.texcoord_index);
                    return seed;
                };
                auto equal_func = [](const tinyobj::index_t &a, const tinyobj::index_t &b) {
                    return a.vertex_index == b.vertex_index &&
                           a.normal_index == b.normal_index &&
                           a.texcoord_index == b.texcoord_index;
                };
                std::unordered_map<tinyobj::index_t, uint32_t, decltype(hash_func), decltype(equal_func)>
                        unique_vertices(face_indices.size(), hash_func, equal_func);

                AssetCpuGeometry cpu_geom;
                auto positions = cpu_geom.vertices.add<glm::vec3>("v:point");
                auto normals = cpu_geom.vertices.add<glm::vec3>("v:normal");
                auto tex_coords = cpu_geom.vertices.add<glm::vec2>("v:texcoord");
                auto indices = cpu_geom.faces.add<uint32_t>("f:indices");

                // De-duplicate vertices just for this sub-mesh.
                for (const auto &index: face_indices) {
                    if (unique_vertices.count(index) == 0) {
                        unique_vertices[index] = static_cast<uint32_t>(positions.vector().size());

                        positions.vector().emplace_back(attrib.vertices[3 * index.vertex_index + 0],
                                                        attrib.vertices[3 * index.vertex_index + 1],
                                                        attrib.vertices[3 * index.vertex_index + 2]);
                        if (index.normal_index >= 0)
                            normals.vector().emplace_back(
                                attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]);
                        if (index.texcoord_index >= 0)
                            tex_coords.vector().emplace_back(
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                attrib.texcoords[2 * index.texcoord_index + 1]);
                    }
                    indices.vector().push_back(unique_vertices[index]);
                }

                // --- Create the Sub-Mesh Asset in the Database ---
                auto &registry = db.get_registry();
                auto entity_id = registry.create();

                std::string submesh_name = shape.name + "_" + (material_idx >= 0
                                                                   ? materials[material_idx].name
                                                                   : "default");
                registry.emplace<AssetName>(entity_id, submesh_name);
                registry.emplace<AssetCpuGeometry>(entity_id, std::move(cpu_geom));

                // Find the correct material ID for this sub-mesh.
                AssetID final_material_id = (material_idx >= 0 && material_idx < material_asset_ids.size())
                                                ? material_asset_ids[material_idx]
                                                : material_asset_ids[0]; // Fallback to first material

                registry.emplace<AssetMetadata>(entity_id, final_material_id);

                // Create the handle for this sub-mesh asset. Its URI is the original file.
                auto deleter = [&db](const AssetID_Data *data) {
                    if (db.get_registry().valid(data->entity_id)) {
                        db.get_registry().destroy(data->entity_id);
                    }
                    delete data;
                };
                AssetID submesh_id(new AssetID_Data{entity_id, original_uri}, deleter);
                // Link this sub-mesh to the parent prefab.
                prefab.child_assets.push_back(submesh_id);
            }
        }
    };
}
