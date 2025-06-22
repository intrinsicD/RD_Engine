#include "AssetManager.h"
#include "MeshAsset.h"
#include "HashUtils.h"
#include "Log.h"

#include <unordered_map>
#include <vector>
#include <cstdint>

// tiny_obj_loader requires this to be defined in one .cpp file
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
    template<>
    struct hash<RDE::Vertex> {
        size_t operator()(const RDE::Vertex& v) const {
            size_t seed = 0;
            // Now we use our robust hash combine utility
            RDE::HashCombine(seed, v.position);
            RDE::HashCombine(seed, v.normal);
            RDE::HashCombine(seed, v.tex_coords);
            return seed;
        }
    };
}

namespace RDE {
    // --- Mesh Specialization ---
    template<>
    AssetHandle AssetManager::load<MeshAsset>(const std::filesystem::path& path) {
        if (asset_registry.contains(path)) {
            return asset_registry.at(path);
        }

        auto mesh = std::make_shared<MeshAsset>();

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str())) {
            RDE_CORE_ERROR("tinyobjloader failed: {} {}", warn, err);
            return INVALID_ASSET_ID;
        }

        if (!warn.empty()) RDE_CORE_WARN("tinyobjloader: {}", warn);

        std::unordered_map<Vertex, uint32_t> unique_vertices{};

        // Combine all shapes from the .obj file into a single MeshAsset
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.tex_coords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                // If this combination of pos/normal/uv is new, add it.
                if (unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = static_cast<uint32_t>(mesh->vertices.size());
                    mesh->vertices.push_back(vertex);
                }
                mesh->indices.push_back(unique_vertices[vertex]);
            }
        }

        AssetHandle handle = NewID();
        asset_registry[path] = handle;
        assets[handle] = mesh;

        RDE_CORE_INFO("Mesh loaded: {}", path.string());
        return handle;
    }
}