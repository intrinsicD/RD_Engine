#pragma once

#include "ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetManager.h"
#include "core/Log.h"
#include "core/Paths.h"

#include <yaml-cpp/yaml.h>

namespace RDE {
    struct PrefabComponentData {
        std::string type; // e.g., "TransformComponent"
        YAML::Node data;  // The YAML node containing the component's fields
    };

    // Represents one entity within the prefab definition.
    struct PrefabEntityData {
        uint32_t local_id = 0;
        std::string name;
        int parent_id = -1; // -1 indicates a root entity within the prefab
        std::vector<PrefabComponentData> components;
    };

    // This is the primary component that gets stored in the AssetDatabase for a .prefab asset.
    struct AssetPrefabData {
        std::string name;
        std::vector<PrefabEntityData> entities;
    };

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
            AssetPrefabData prefab_data;

            if (data["name"]) {
                prefab_data.name = data["name"].as<std::string>();
            } else {
                // Use the filename as a fallback name
                prefab_data.name = std::filesystem::path(uri).stem().string();
            }

            // --- THE CORE PARSING LOGIC ---
            if (data["entities"] && data["entities"].IsSequence()) {
                for (const auto &entity_node: data["entities"]) {
                    PrefabEntityData entity_data;

                    // Parse the entity's direct properties
                    entity_data.local_id = entity_node["id"].as<uint32_t>();
                    entity_data.name = entity_node["name"].as<std::string>("Entity"); // Default to "Entity" if no name
                    entity_data.parent_id = entity_node["parent"].as<int>(-1); // Default to -1 if no parent

                    // Parse the components for this entity
                    if (entity_node["components"] && entity_node["components"].IsSequence()) {
                        for (const auto &comp_node: entity_node["components"]) {
                            PrefabComponentData component_data;
                            component_data.type = comp_node["type"].as<std::string>();
                            component_data.data = comp_node; // Store the whole component node for later
                            //TODO go a different route here, maybe use a map of component types to data?#
                            //defenitly construct the asset as a prototype, which can then be referenced or partially copied (overwritten) to the scene.
                            entity_data.components.push_back(std::move(component_data));
                        }
                    }
                    prefab_data.entities.push_back(std::move(entity_data));
                }
            }

            // --- Store the fully parsed data in the AssetDatabase ---
            auto entity_id = db.get_registry().create();

            // Emplace the main data component
            db.get_registry().emplace<AssetPrefabData>(entity_id, std::move(prefab_data));

            // Emplace metadata
            db.get_registry().emplace<AssetFilepath>(entity_id, uri);
            // We can also emplace the name for easy lookup if needed
            db.get_registry().emplace<AssetName>(entity_id, data["name"].as<std::string>());

            RDE_CORE_TRACE("PrefabLoader: Successfully loaded and parsed prefab '{}'", uri);
            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".prefab"};
        }
    };
}