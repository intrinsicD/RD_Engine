#include "assets/MeshObjLoader.h"
#include "assets/AssetComponentTypes.h"
#include "tiny_obj_loader.h"
#include "core/Log.h"

#include <glm/glm.hpp>
#include <filesystem>

namespace RDE {
    std::vector<std::string> MeshObjLoader::get_dependencies(const std::string &uri) const {
        // OBJ files typically reference materials in MTL files.
        // This is a simplified example; you might want to parse the file to find actual dependencies.
        return {uri + ".mtl"};
    }


    AssetID MeshObjLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig reader_config;

        // Set the base path for relative paths in the OBJ file
        reader_config.mtl_search_path = std::filesystem::path(uri).parent_path().string();

        if (!reader.ParseFromFile(uri, reader_config)) {
            if (!reader.Error().empty()) {
                RDE_CORE_ERROR("Failed to load OBJ file '{}': {}", uri, reader.Error());
            }
            return nullptr; // Return an invalid AssetID on failure
        }

        const auto &attrib = reader.GetAttrib();
        const auto &shapes = reader.GetShapes();
        const auto &materials = reader.GetMaterials();

        // Create a new entity in the AssetDatabase
        auto &registry = db.get_registry();
        auto entity_id = registry.create();

        // Populate the geometry data into the AssetCpuGeometry component
        AssetCpuGeometry geometry;
        if (!attrib.vertices.empty()) {
            auto positions = geometry.vertices.get_or_add<glm::vec3>("positions");
            geometry.vertices.resize(attrib.vertices.size() / 3);
            // Add vertices
            for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
                positions[i] = glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
            }
        }


        if (!attrib.texcoords.empty()) {
            auto texcoords = geometry.edges.get_or_add<glm::vec2>("texcoords");
            geometry.edges.resize(attrib.texcoords.size() / 2);
            // Add texture coordinates
            for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
                texcoords[i] = glm::vec2(attrib.texcoords[i], attrib.texcoords[i + 1]);
            }
        }


        // Add faces and materials
        for (const auto &shape: shapes) {
            for (const auto &index: shape.mesh.indices) {
                geometry.faces.add<uint32_t>(index.vertex_index);
                if (index.normal_index >= 0) {
                    geometry.tets.add<uint32_t>(index.normal_index);
                }
                if (index.texcoord_index >= 0) {
                    geometry.tets.add<uint32_t>(index.texcoord_index);
                }
            }
            // Optionally handle materials here
            if (shape.mesh.material_ids.size() > 0) {
            }

            std::vector<std::string> MeshObjLoader::get_supported_extensions() const {
                return {".obj"};
            }
        }
    }
}