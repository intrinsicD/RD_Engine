#include "utils/AssetManagerUtils.h"
#include "../../../log/include/Log.h"
#include <tiny_obj_loader.h>

namespace RDE{
    std::shared_ptr<GeometryData> load_obj(const std::string &path){
        //load obj file using tinyobjloader

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = std::filesystem::path(path).parent_path().string();
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(path, reader_config)) {
            if (!reader.Error().empty()) {
                RDE_CORE_ERROR("TinyObjLoader error: {}", reader.Error());
            }
            return nullptr; // Return nullptr if loading failed
        }
        if (!reader.Warning().empty()) {
            RDE_CORE_WARN("TinyObjLoader warning: {}", reader.Warning());
        }
        const auto &attrib = reader.GetAttrib();
        const auto &shapes = reader.GetShapes();
        const auto &materials = reader.GetMaterials();
        std::shared_ptr<GeometryData> geometry_data = std::make_shared<GeometryData>();
        //Vertices are position, normal, and texture coordinates in a struct.
        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex;
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
                } else {
                    vertex.normal = {0.0f, 0.0f, 0.0f}; // Default normal if not available
                }
                if (index.texcoord_index >= 0) {
                    vertex.tex_coords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                } else {
                    vertex.tex_coords = {0.0f, 0.0f}; // Default texcoord if not available
                }
                geometry_data->vertices.push_back(vertex);
            }
        }
        geometry_data->indices.reserve(shapes.size() * 3); // Assuming triangles

        for (const auto &shape : shapes) {
            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
                geometry_data->indices.push_back(shape.mesh.indices[i].vertex_index);
                geometry_data->indices.push_back(shape.mesh.indices[i + 1].vertex_index);
                geometry_data->indices.push_back(shape.mesh.indices[i + 2].vertex_index);
            }
        }
        RDE_CORE_INFO("Loaded OBJ file: {} with {} vertices and {} indices", path, geometry_data->vertices.size(), geometry_data->indices.size());
        return geometry_data;
    }
    std::shared_ptr<GeometryData> load_gltf(const std::string &path){

    }
    std::shared_ptr<GeometryData> load_stl(const std::string &path){

    }
    std::shared_ptr<GeometryData> load_off(const std::string &path){

    }
    std::shared_ptr<GeometryData> load_ply(const std::string &path){

    }
}