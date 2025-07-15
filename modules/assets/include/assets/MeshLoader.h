#pragma once

#include "AssetDatabase.h"
#include "AssetComponentTypes.h"
#include "ILoader.h"
#include "AssetConcepts.h"
#include "core/Log.h"

#include <tiny_obj_loader.h> // Include the tinyobjloader library for mesh loading
#include <glm/glm.hpp>
#include <entt/resource/loader.hpp>
#include <filesystem>

namespace RDE {

    // A concrete loader for AssetCpuTexture using the stb_image library.
    // It inherits from the entt helper to satisfy the loader concept.
    // The second template argument is the type of resource it returns.
    struct MeshLoader final: public ILoader{
        AssetID load(const std::string &uri, AssetDatabase &db) const override{
            RDE_CORE_TRACE("MeshLoader: Loading mesh from '{}'...", uri);

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            // 1. --- Perform the actual File I/O and Parsing ---
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, uri.c_str())) {
                RDE_CORE_ERROR("MeshLoader: Failed to load mesh '{}'. Warn: {}, Err: {}", uri, warn, err);
                return nullptr;
            }

            if (!warn.empty()) {
                RDE_CORE_WARN("MeshLoader: Warning while loading '{}': {}", uri, warn);
            }

            // 2. --- Create the AssetCpuGeometry and de-duplicate vertices ---
            auto hash_func = [](const tinyobj::index_t& i) {
                size_t seed = 0;
                hash_combine(seed, i.vertex_index);
                hash_combine(seed, i.normal_index);
                hash_combine(seed, i.texcoord_index);
                return seed;
            };

            auto equal_func = [](const tinyobj::index_t& a, const tinyobj::index_t& b) {
                return a.vertex_index == b.vertex_index &&
                       a.normal_index == b.normal_index &&
                       a.texcoord_index == b.texcoord_index;
            };

            std::unordered_map<tinyobj::index_t, uint32_t, decltype(hash_func), decltype(equal_func)>
                    unique_vertices(1024, hash_func, equal_func);

            // We'll create typed buffers within the PropertyContainer
            std::vector<glm::vec3> temp_positions;
            std::vector<glm::vec3> temp_normals;
            std::vector<glm::vec2> temp_tex_coords;
            std::vector<uint32_t> temp_indices;

            for (const auto &shape: shapes) {
                for (const auto &index: shape.mesh.indices) {
                    if (unique_vertices.count(index) == 0) {
                        unique_vertices[index] = static_cast<uint32_t>(temp_positions.size());

                        temp_positions.emplace_back(attrib.vertices[3 * index.vertex_index + 0],
                                                    attrib.vertices[3 * index.vertex_index + 1],
                                                    attrib.vertices[3 * index.vertex_index + 2]);

                        if (index.normal_index >= 0) {
                            temp_normals.emplace_back(attrib.normals[3 * index.normal_index + 0],
                                                      attrib.normals[3 * index.normal_index + 1],
                                                      attrib.normals[3 * index.normal_index + 2]);
                        }

                        if (index.texcoord_index >= 0) {
                            temp_tex_coords.emplace_back(
                                    attrib.texcoords[2 * index.texcoord_index + 0],
                                    attrib.texcoords[2 * index.texcoord_index + 1]
                            );
                        }
                    }
                    temp_indices.push_back(unique_vertices[index]);
                }
            }

            // 3. --- Populate the AssetDatabase ---
            auto &registry = db.get_registry();
            auto entity_id = registry.create();

            AssetCpuGeometry cpu_geom;
            auto positions = cpu_geom.vertices.add<glm::vec3>("v:point");
            auto indices = cpu_geom.faces.add<uint32_t>("f:indices");

            size_t num_vertices = temp_positions.size();
            size_t num_faces = temp_indices.size() / 3;

            cpu_geom.vertices.resize(num_vertices);
            cpu_geom.faces.resize(num_faces);

            positions.vector() = std::move(temp_positions);
            indices.vector() = std::move(temp_indices);
            if (!temp_normals.empty()) {
                auto normals = cpu_geom.vertices.add<glm::vec3>("v:normal");
                normals.vector() = std::move(temp_normals);
            }
            if (!temp_tex_coords.empty()) {
                auto tex_coords = cpu_geom.vertices.add<glm::vec2>("v:texcoord0");
                tex_coords.vector() = std::move(temp_tex_coords);
            }

            // CORRECTED: Emplace AssetCpuGeometry
            registry.emplace<AssetCpuGeometry>(entity_id, std::move(cpu_geom));
            registry.emplace<AssetFilepath>(entity_id, uri);
            std::string name = std::filesystem::path(uri).filename().string();
            registry.emplace<AssetName>(entity_id, name);

            RDE_CORE_INFO("MeshLoader: Successfully loaded '{}' (#V: {} #I: {}).", name, num_vertices, num_faces);

            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".obj", ".gltf", ".glb"}; // Add more supported extensions as needed
        }

    private:
        // Helper for the hash map
        template<class T>
        static void hash_combine(std::size_t &seed, const T &v) {
            std::hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };

} // namespace RDE