#pragma once

#include "ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetManager.h"
#include "core/Log.h"
#include "core/YamlHelper.h"
#include "components/NameTagComponent.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/RenderableComponent.h"
#include "components/HierarchyComponent.h"


namespace RDE {
    class PrefabLoader final : public ILoader {
    public:
        PrefabLoader() = default;

        // --- Fast Dependency Discovery ---
        std::vector<std::string> get_dependencies(const std::string &uri) const override {
            std::vector<std::string> dependencies;
            YAML::Node data = YAML::LoadFile(uri);

            if (data["materials"]) {
                for (const auto &material_node: data["materials"]) {
                    dependencies.push_back(material_node.as<std::string>());
                }
            }
            if(data["meshes"]){
                for (const auto &mesh_node: data["meshes"]) {
                    dependencies.push_back(mesh_node.as<std::string>());
                }
            }
            if(data["geometry"]){
                for (const auto &geometry_node: data["geometry"]) {
                    dependencies.push_back(geometry_node.as<std::string>());
                }
            }
            return dependencies;
        }

        // --- The Actual Loading Function ---
        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override {
            (void) manager; // Not needed for this loader's logic

            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception &e) {
                RDE_CORE_ERROR("PrefabLoader: Failed to parse YAML file '{}'. Error: {}", uri, e.what());
                return nullptr;
            }

            std::vector<AssetID> loaded_materials;
            std::vector<AssetID> loaded_meshes;
            // --- A. Parse the Prefab Metadata ---
            if(data["materials"]){
                for (const auto &material_node: data["materials"]) {
                    auto material_uri = material_node.as<std::string>();
                    if (AssetID material_id = manager.get_loaded_asset(material_uri)) {
                        loaded_materials.push_back(material_id);
                    } else {
                        RDE_CORE_ERROR("PrefabLoader: Material '{}' not found in cache for prefab '{}'", material_uri, uri);
                    }
                }
            }
            if(data["meshes"]){
                for (const auto &mesh_node: data["meshes"]) {
                    auto mesh_uri = mesh_node.as<std::string>();
                    if (AssetID mesh_id = manager.get_loaded_asset(mesh_uri)) {
                        loaded_meshes.push_back(mesh_id);
                    } else {
                        RDE_CORE_ERROR("PrefabLoader: Mesh '{}' not found in cache for prefab '{}'", mesh_uri, uri);
                    }
                }
            }

            // This is the C++ object we will populate.

            std::string prefab_name;
            if (data["name"]) {
                prefab_name = data["name"].as<std::string>();
            } else {
                // Use the filename as a fallback name
                prefab_name = std::filesystem::path(uri).stem().string();
            }

            struct AssetHierarchyNode{
                std::string name;
                TransformLocal transform; // Local transform for this node
                RenderableComponent mesh; // Geometry for this node, if any
                MaterialComponent material; // Material for this node, if any
                AssetHierarchy hierarchy;
            };

            std::unordered_map<int, AssetHierarchyNode> nodes;

            if(data["nodes"]){
                int i = 0;
                for(const auto &node: data["nodes"]){
                    // Parse each node in the prefab
                    AssetHierarchyNode hierarchy_node;
                    if(node["name"]) {
                        hierarchy_node.name = node["name"].as<std::string>();
                    } else {
                        hierarchy_node.name = "Unnamed Node";
                    }

                    if(node["transform"]) {
                        const auto &transform_node = node["transform"];
                        hierarchy_node.transform.translation = transform_node["translation"].as<glm::vec3>();
                        hierarchy_node.transform.orientation = glm::quat(transform_node["orientation"].as<glm::vec4>(glm::vec4(1, 0, 0, 0)));
                        hierarchy_node.transform.scale = transform_node["scale"].as<glm::vec3>(glm::vec3(1.0f, 1.0f, 1.0f));
                    } else {
                        hierarchy_node.transform = TransformLocal(); // Default identity transform
                    }
                    if(node["mesh"]) {
                        auto mesh_index = node["mesh"].as<int>();
                        if(mesh_index >= 0 && mesh_index < loaded_meshes.size()) {
                            hierarchy_node.mesh.geometry_id = loaded_meshes[mesh_index];
                        } else {
                            RDE_CORE_ERROR("PrefabLoader: Mesh index {} out of bounds for prefab '{}'", mesh_index, uri);
                        }
                    }
                    if(node["material"]) {
                        auto material_index = node["material"].as<int>();
                        if(material_index >= 0 && material_index < loaded_materials.size()) {
                            hierarchy_node.material.material_asset_id = loaded_materials[material_index];
                        } else {
                            RDE_CORE_ERROR("PrefabLoader: Material index {} out of bounds for prefab '{}'", material_index, uri);
                        }
                    }
                    if(node["hierarchy"]) {
                        const auto &hierarchy_node_data = node["hierarchy"];
                        if(hierarchy_node_data["parent"]) {
                            auto parent_index = hierarchy_node_data["parent"].as<int>();
                            if(parent_index >= 0 || parent_index < nodes.size()) {
                                hierarchy_node.hierarchy.parent = hierarchy_node_data["parent"].as<int>();
                            }

                        }
                        if(hierarchy_node_data["children"]) {
                            for(const auto &child_index: hierarchy_node_data["children"]) {
                                hierarchy_node.children_indices.push_back(child_index.as<int>());
                            }
                        }
                    }
                    nodes[i] = hierarchy_node;
                    ++i;
                }
            }
            int root;
            if(data["scene"]){
                auto scene_node = data["scene"];
                if(scene_node["root"]) {
                    root = scene_node["root"].as<int>();
                } else {
                    RDE_CORE_ERROR("PrefabLoader: No root node specified in prefab '{}'", uri);
                    return nullptr;
                }
            }

            // --- Store the fully parsed data in the AssetDatabase ---
            auto &asset_registry = db.get_registry();
            std::unordered_map<int, entt::entity> index_entity_map;
            for(const auto &node_pair : nodes) {
                if(node_pair.first < 0 || node_pair.first >= nodes.size()) {
                    RDE_CORE_ERROR("PrefabLoader: Root index {} out of bounds for prefab '{}'", node_pair.first, uri);
                    continue;
                }
                auto &node = node_pair.second;

                auto entity_id = asset_registry.create();
                index_entity_map[node_pair.first] = entity_id;

                asset_registry.emplace<TransformLocal>(entity_id, node.transform);
                asset_registry.emplace<RenderableComponent>(entity_id, node.mesh);
                asset_registry.emplace<MaterialComponent>(entity_id, node.material);
                asset_registry.emplace<AssetHierarchy>(entity_id);
                // Emplace metadata
                db.get_registry().emplace<AssetFilepath>(entity_id, uri);
                // We can also emplace the name for easy lookup if needed
                db.get_registry().emplace<AssetName>(entity_id, node.name);
            }

            entt::entity root_id = index_entity_map[root];


            RDE_CORE_TRACE("PrefabLoader: Successfully loaded and parsed prefab '{}'", uri);
            return std::make_shared<AssetID_Data>(root_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".prefab"};
        }
    };
}
