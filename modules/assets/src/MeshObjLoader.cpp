#include "assets/MeshObjLoader.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace tinyobj {
    struct index_t_hash {
        std::size_t operator()(const index_t &i) const {
            std::size_t h1 = std::hash<int>()(i.vertex_index);
            std::size_t h2 = std::hash<int>()(i.normal_index);
            std::size_t h3 = std::hash<int>()(i.texcoord_index);
            // A common way to combine hashes
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    bool operator==(const index_t &a, const index_t &b) {
        return a.vertex_index == b.vertex_index &&
               a.normal_index == b.normal_index &&
               a.texcoord_index == b.texcoord_index;
    }
}

namespace RDE {
    std::vector<std::string> MeshObjLoader::get_dependencies(const std::string &uri) const {
        std::vector<std::string> dependencies;
        std::ifstream file(uri);
        if (!file.is_open()) {
            RDE_CORE_WARN("get_dependencies could not open file: {}", uri);
            return dependencies;
        }

        std::filesystem::path base_path = std::filesystem::path(uri).parent_path();
        std::string line;
        while (std::getline(file, line)) {
            // Trim whitespace for robustness
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            if (line.rfind("mtllib", 0) == 0) { // Check if the line starts with "mtllib"
                std::stringstream ss(line);
                std::string keyword, mtl_path_str;
                ss >> keyword >> std::ws; // Read keyword, consume whitespace
                std::getline(ss, mtl_path_str);

                if (!mtl_path_str.empty()) {
                    // Construct the full path relative to the OBJ file
                    std::string full_path = (base_path / mtl_path_str).lexically_normal().string();
                    dependencies.push_back(full_path);
                }
            }
        }
        return dependencies;
    }


    AssetID MeshObjLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = std::filesystem::path(uri).parent_path().string();
        reader_config.triangulate = true; // IMPORTANT: Ensure we always get triangles.

        if (!reader.ParseFromFile(uri, reader_config)) {
            if (!reader.Error().empty()) {
                RDE_CORE_ERROR("Failed to load OBJ file '{}': {}", uri, reader.Error());
            }
            return nullptr;
        }

        const auto &attrib = reader.GetAttrib();
        const auto &shapes = reader.GetShapes();
        const auto &materials = reader.GetMaterials();

        // -- This is the object we will populate and eventually emplace into the asset entity --
        AssetCpuGeometry geometry;
        // Get typed handles to the property arrays we will populate.
        auto positions = geometry.vertices.add<glm::vec3>("v:point");
        auto normals = geometry.vertices.add<glm::vec3>("v:normal", glm::vec3(0.0f, 1.0f, 0.0f)); // Default normal
        auto texCoords = geometry.vertices.add<glm::vec2>("v:texcoord");

        // This will temporarily hold ALL indices from all shapes before we form faces.
        std::vector<uint32_t> masterIndexBuffer;

        std::unordered_map<tinyobj::index_t, uint32_t, tinyobj::index_t_hash> uniqueVertices{};

        //can we just copy all values directly? and then only set the subview offsets and indices?

        for (const auto &shape: shapes) {
            AssetGeometrySubView subGeom{};
            subGeom.index_offset = static_cast<uint32_t>(masterIndexBuffer.size());

            if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
                subGeom.material_name = materials[shape.mesh.material_ids[0]].name;
            } else {
                subGeom.material_name = "DefaultMaterial";
            }

            // Iterate over the faces of the shape, creating the unified index buffer
            for (const auto &index: shape.mesh.indices) {
                if (uniqueVertices.count(index) == 0) {
                    // This unique vertex doesn't exist yet, so we add its data
                    uniqueVertices[index] = static_cast<uint32_t>(positions.vector().size());
                    geometry.vertices.push_back();

                    positions.vector().back() = glm::vec3(
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                    );

                    if (index.normal_index >= 0) {
                        normals.vector().back() = glm::vec3(
                                attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]
                        );
                    }

                    if (index.texcoord_index >= 0) {
                        texCoords.vector().back() = glm::vec2(
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                1.0f -
                                attrib.texcoords[2 * index.texcoord_index + 1] // Flip V coordinate for Vulkan/OpenGL
                        );
                    }
                }
                // Add the unified index to our temporary master list
                masterIndexBuffer.push_back(uniqueVertices[index]);
            }

            subGeom.index_count = static_cast<uint32_t>(masterIndexBuffer.size()) - subGeom.index_offset;
            if (subGeom.index_count > 0) {
                geometry.subviews.push_back(subGeom);
            }
        }

        // -- Post-Process and Finalize --

        // Pad missing attributes to ensure all vertex property arrays have the same size.
        size_t vertexCount = positions.vector().size();
        if (normals.vector().size() < vertexCount) {
            normals.vector().resize(vertexCount, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (texCoords.vector().size() < vertexCount) {
            texCoords.vector().resize(vertexCount, glm::vec2(0.0f));
        }

        // Now, correctly convert the master index buffer into faces (triangles)
        if (!masterIndexBuffer.empty()) {
            auto faces = geometry.faces.add<glm::ivec3>("f:tris");
            geometry.faces.resize(masterIndexBuffer.size() / 3);
            for (size_t i = 0; i < geometry.faces.size(); ++i) {
                faces[i] = glm::ivec3(
                        masterIndexBuffer[3 * i + 0],
                        masterIndexBuffer[3 * i + 1],
                        masterIndexBuffer[3 * i + 2]
                );
            }
        }

        if (geometry.getVertexCount() == 0 || geometry.faces.empty()) {
            RDE_CORE_WARN("Loaded empty or invalid mesh from '{}'", uri);
            return nullptr;
        }

        // -- Create the asset entity and emplace its data --
        auto &asset_registry = db.get_registry();
        entt::entity entity_id = asset_registry.create();

        asset_registry.emplace<AssetFilepath>(entity_id, uri);
        asset_registry.emplace<AssetName>(entity_id, std::filesystem::path(uri).filename().string());
        asset_registry.emplace<AssetCpuGeometry>(entity_id, std::move(geometry));

        RDE_CORE_INFO("MeshObjLoader: Successfully populated asset for '{}'", uri);
        return std::make_shared<AssetID_Data>(entity_id, uri);
    }

    std::vector<std::string> MeshObjLoader::get_supported_extensions() const {
        return {".obj"};
    }
}
