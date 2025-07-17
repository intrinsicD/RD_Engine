#pragma once

#include "ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetManager.h"
#include "core/Log.h"
#include "components/NameTagComponent.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/RenderableComponent.h"
#include "components/HierarchyComponent.h"


#include <yaml-cpp/yaml.h>


namespace RDE {
    class PrefabLoader final : public ILoader {
    public:
        PrefabLoader() = default;

        // --- Fast Dependency Discovery ---
        std::vector<std::string> get_dependencies(const std::string &uri) const override {
            std::vector<std::string> dependencies;
            YAML::Node data = YAML::LoadFile(uri);

            if (data["entities"]) {
                for (const auto &entity_node: data["entities"]) {
                    if (entity_node["components"]) {
                        for (const auto &comp_node: entity_node["components"]) {
                            // Scan for known asset links
                            if (comp_node["mesh"]) {
                                dependencies.push_back(comp_node["mesh"].as<std::string>());
                            }
                            if (comp_node["material"]) {
                                dependencies.push_back(comp_node["material"].as<std::string>());
                            }
                            // Add other dependency types here (textures, etc.)
                        }
                    }
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

            // This is the C++ object we will populate.
            std::string prefab_name;
            TransformLocal prefab_transform; // Default to identity transform
            MaterialComponent prefab_material;
            RenderableComponent prefab_geometry;
            AssetHierarchy prefab_hierarchy;

            if (data["name"]) {
                prefab_name = data["name"].as<std::string>();
            } else {
                // Use the filename as a fallback name
                prefab_name = std::filesystem::path(uri).stem().string();
            }

            struct LocalHierarchy {
                uint32_t parent_id;
                std::vector<uint32_t> children;
            }local_hierarchy;

            // --- THE CORE PARSING LOGIC ---
            if (data["entities"] && data["entities"].IsSequence()) {
                for (const auto &entity_node: data["entities"]) {
                    auto entity_id = db.get_registry().create();

                    // Parse the entity's direct properties
                    auto local_id = entity_node["id"].as<uint32_t>();
                    auto name = entity_node["name"].as<std::string>("Entity"); // Default to "Entity" if no name

                    // Parse the components for this entity
                    if (entity_node["components"] && entity_node["components"].IsSequence()) {
                        for (const auto &comp_node: entity_node["components"]) {
                            auto type = comp_node["type"].as<std::string>();
                            if (type == "TransformComponent") {
                                prefab_transform.translation = comp_node["translation"].as<glm::vec3>();
                            }
                            if (type == "RenderableComponent") {
                                auto material_uri = comp_node["material"].as<std::string>();
                                prefab_material.material_asset_id = manager.get_loaded_asset(material_uri);
                                auto geometry_uri = comp_node["geometry"].as<std::string>();
                                prefab_geometry.geometry_id = manager.get_loaded_asset(geometry_uri);
                            }
                            if (type == "HierarchyComponent") {
                                // Parse hierarchy information and resolve it after all entities are loaded as assets
                                auto local_parent_id = comp_node["parent"].as<uint32_t>(0); // Default to 0 (no parent)
                                local_hierarchy.parent_id = local_parent_id;
                                if (comp_node["children"] && comp_node["children"].IsSequence()) {
                                    for (const auto &child_id: comp_node["children"]) {
                                        local_hierarchy.children.push_back(child_id.as<uint32_t>());
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // --- Store the fully parsed data in the AssetDatabase ---
            auto entity_id = db.get_registry().create();

            // Emplace the main data component
            db.get_registry().emplace<AssetPrefabData>(entity_id, std::move(prefab_data));

            // Emplace metadata
            db.get_registry().emplace<AssetFilepath>(entity_id, uri);
            // We can also emplace the name for easy lookup if needed
            db.get_registry().emplace<AssetName>(entity_id, prefab_name);

            RDE_CORE_TRACE("PrefabLoader: Successfully loaded and parsed prefab '{}'", uri);
            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".prefab"};
        }
    };
}
