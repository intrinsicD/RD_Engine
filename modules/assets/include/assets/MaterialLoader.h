// assets/loaders/MaterialLoader.h (Refactored)
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "core/Paths.h"
#include "core/Log.h"
#include "core/YamlHelper.h"

#include <filesystem>

namespace RDE {
    // Forward-declare so we don't need to include AssetManager.h in a header
    class AssetManager;

    class MaterialLoader final : public ILoader {
    public:
        // No longer needs to store a pointer to the manager.
        MaterialLoader() = default;

        // --- 1. NEW: Fast Dependency Discovery ---
        std::vector<std::string> get_dependencies(const std::string& uri) const override {
            std::vector<std::string> dependencies;
            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception& e) {
                RDE_CORE_ERROR("MaterialLoader (get_dependencies): Failed to parse '{}'. Error: {}", uri, e.what());
                return dependencies; // Return empty on failure
            }

            // A. Add the shader as a dependency
            if (data["shader"]) {
                dependencies.push_back(data["shader"].as<std::string>());
            }

            // B. Add all textures as dependencies
            if (data["textures"]) {
                auto base_path = get_asset_path().value_or(".");
                for (const auto& texture_node : data["textures"]) {
                    auto texture_path = texture_node.second.as<std::string>();
                    // Resolve the relative path to a full, canonical URI
                    std::string full_texture_path = (std::filesystem::path(base_path) / texture_path).string();
                    std::replace(full_texture_path.begin(), full_texture_path.end(), '\\', '/'); // Normalize
                    dependencies.push_back(full_texture_path);
                }
            }

            return dependencies;
        }

        // --- 2. REVISED: The Actual Loading Function ---
        AssetID load_asset(const std::string& uri, AssetDatabase& db, AssetManager& manager) const override {
            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception& e) {
                RDE_CORE_ERROR("MaterialLoader (load_asset): Failed to parse '{}'. Error: {}", uri, e.what());
                return nullptr;
            }

            AssetCpuMaterial cpu_mat;

            if (data["name"]) cpu_mat.name = data["name"].as<std::string>();
            if (data["shader"]) cpu_mat.shader_path = data["shader"].as<std::string>();

            // Pipeline state parsing remains the same
            if (data["pipeline"]) {
                const auto& p_node = data["pipeline"];
                auto cullModeStr = p_node["cullMode"].as<std::string>("Back");
                if (cullModeStr == "None") cpu_mat.cull_mode = RAL::CullMode::None;
                else if (cullModeStr == "Front") cpu_mat.cull_mode = RAL::CullMode::Front;
                else cpu_mat.cull_mode = RAL::CullMode::Back;
                cpu_mat.depth_test = p_node["depthTest"].as<bool>(true);
                cpu_mat.depth_write = p_node["depthWrite"].as<bool>(true);
            }

            // Parameter parsing remains the same
            if (data["parameters"]) {
                const auto& p_node = data["parameters"];
                if (p_node["baseColor"]) cpu_mat.vector_params["baseColor"] = p_node["baseColor"].as<glm::vec4>();
                cpu_mat.float_params["metalness"] = p_node["metalness"].as<float>(0.0f);
                cpu_mat.float_params["roughness"] = p_node["roughness"].as<float>(1.0f);
            }

            // --- C. CRITICAL CHANGE: Handling Texture Dependencies ---
            // We NO LONGER call `manager.load()`. The AssetManager has already ensured
            // dependencies are loaded by the time this function is called.
            // We just need to retrieve the AssetID from the manager's cache.
            if (data["textures"]) {
                auto base_path = get_asset_path().value_or(".");
                for (const auto& texture_node : data["textures"]) {
                    auto sampler_name = texture_node.first.as<std::string>();
                    auto texture_path = texture_node.second.as<std::string>();
                    std::string full_texture_path = (std::filesystem::path(base_path) / texture_path).string();
                    std::replace(full_texture_path.begin(), full_texture_path.end(), '\\', '/'); // Normalize

                    // Assumes a new synchronous get function on AssetManager for loaded assets.
                    if (AssetID texture_id = manager.get_loaded_asset(full_texture_path)) {
                        cpu_mat.texture_asset_ids[sampler_name] = texture_id;
                    } else {
                        // This case indicates a logic error in the dependency graph or loader.
                        RDE_CORE_ERROR("MaterialLoader: Dependency '{}' for material '{}' was not loaded!", full_texture_path, uri);
                    }
                }
            }

            // Populating the database is identical
            auto& registry = db.get_registry();
            auto entity_id = registry.create();

            auto name = cpu_mat.name;
            registry.emplace<AssetCpuMaterial>(entity_id, std::move(cpu_mat));
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, name);

            RDE_CORE_TRACE("MaterialLoader: Successfully populated asset for '{}'", name);

            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        // --- 3. Unchanged ---
        std::vector<std::string> get_supported_extensions() const override {
            return {".mat"};
        }
    };
}
