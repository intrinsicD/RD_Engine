#pragma once

#include "assets/ILoader.h"
#include "assets/AssetManager.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"

#include <tiny_obj_loader.h>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

#include <fstream>
#include <filesystem>
#include <map>
#include <vector>

namespace RDE {
    class MeshLoader final : public ILoader {
    public:
        // The loader needs a pointer to the AssetManager to recursively load
        // the material assets it generates on the fly.
        explicit MeshLoader(AssetManager *asset_manager)
            : m_asset_manager(asset_manager) {
        }

        // This is the main entry point called by AssetManager for .obj files.
        // It creates a "Prefab" asset that contains links to all the sub-assets (meshes).
        AssetID load(const std::string &uri, AssetDatabase &db) const override {
            RDE_CORE_INFO("MeshLoader: Loading prefab from '{}'...", uri);

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            std::string base_dir = std::filesystem::path(uri).parent_path().string();

            // Load the obj file, which also triggers parsing of the associated .mtl file.
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, uri.c_str(), base_dir.c_str(), true)) {
                RDE_CORE_ERROR("MeshLoader: Failed to load '{}'. Err: {}", uri, err);
                return nullptr;
            }
            if (!warn.empty()) {
                RDE_CORE_WARN("MeshLoader: Warning for '{}': {}", uri, warn);
            }

            // --- 1. Process Materials ---
            // Convert each tinyobj::material_t into an engine-native .mat asset.
            // This generates the files on disk and then loads them to get valid AssetIDs.
            std::vector<AssetID> material_asset_ids;
            for (const auto &mtl: materials) {
                material_asset_ids.push_back(process_mtl_as_asset(mtl, base_dir, db));
            }

            // If the model had no materials, we need a fallback default material.
            // Ensure you have "assets/materials/default.mat" created.
            if (material_asset_ids.empty()) {
                RDE_CORE_WARN("MeshLoader: No materials found for '{}'. Using default material.", uri);
                material_asset_ids.push_back(m_asset_manager->load(get_asset_path().value() / "materials" / "default.mat"));
            }

            // --- 2. Create the main Prefab Asset ---
            auto &registry = db.get_registry();
            auto prefab_entity_id = registry.create();
            registry.emplace<AssetFilepath>(prefab_entity_id, uri);
            registry.emplace<AssetName>(prefab_entity_id, std::filesystem::path(uri).stem().string());
            auto &prefab_comp = registry.emplace<AssetPrefab>(prefab_entity_id);

            // --- 3. Process Geometry ---
            // Iterate through all shapes in the file and create sub-mesh assets from them.
            for (const auto &shape: shapes) {
                create_submeshes_from_shape(prefab_comp, shape, attrib, materials, material_asset_ids, db, uri);
            }

            RDE_CORE_TRACE("MeshLoader: Processed prefab '{}' with {} sub-meshes.", uri,
                           prefab_comp.child_assets.size());
            // The receipt points to the main prefab asset.
            return std::make_shared<AssetID_Data>(prefab_entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".obj"};
        }

    private:
        AssetManager *m_asset_manager;

        // --- HASHING HELPER ---
        template<class T>
        static void hash_combine(std::size_t &seed, const T &v) {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // --- MATERIAL PROCESSING ---
        // Takes a tinyobj material, generates a .mat file, and loads it to get an AssetID.
        AssetID process_mtl_as_asset(const tinyobj::material_t &mtl, const std::string &obj_base_dir,
                                     AssetDatabase &db) const {
            std::filesystem::path material_path = get_asset_path().value() / "materials" / (mtl.name + ".mat");
            std::filesystem::create_directories(material_path.parent_path());

            YAML::Node mat_yaml;
            mat_yaml["name"] = mtl.name;
            mat_yaml["shader"] = "shaders/basic_lit.glsl";
            mat_yaml["pipeline"]["cullMode"] = "Back";
            mat_yaml["pipeline"]["depthTest"] = true;
            mat_yaml["pipeline"]["depthWrite"] = true;
            mat_yaml["parameters"]["baseColor"] = std::vector<float>{
                mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2], 1.0f
            };

            float roughness = 1.0f - (std::clamp(mtl.shininess, 0.f, 1000.f) / 1000.f);
            mat_yaml["parameters"]["roughness"] = roughness;
            mat_yaml["parameters"]["metalness"] = (mtl.specular[0] > 0.5f) ? 0.9f : 0.1f;

            if (!mtl.diffuse_texname.empty()) {
                mat_yaml["textures"]["albedoMap"] = mtl.diffuse_texname;
            }

            std::ofstream fout(material_path);
            fout << mat_yaml;
            fout.close();

            return m_asset_manager->load(material_path.string());
        }

        // --- GEOMETRY PROCESSING ---
        // Processes a single shape, splitting it into multiple mesh assets if it uses multiple materials.
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
                auto tex_coords = cpu_geom.vertices.add<glm::vec2>("v:texcoord0");
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
