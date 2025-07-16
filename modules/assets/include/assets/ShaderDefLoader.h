// assets/loaders/ShaderDefLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"

#include <yaml-cpp/yaml.h>

namespace RDE {
    class ShaderDefLoader final : public ILoader {
    public:
        ShaderDefLoader() = default;

        // A .shaderdef file's dependencies (the GLSL sources) are resolved at build-time,
        // not runtime. From the AssetManager's perspective, it has no dependencies.
        std::vector<std::string> get_dependencies(const std::string& uri) const override {
            (void)uri;
            return {};
        }

        AssetID load_asset(const std::string& uri, AssetDatabase& db, AssetManager& manager) const override {
            (void)manager; // No other assets to look up.

            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception& e) {
                RDE_CORE_ERROR("ShaderDefLoader: Failed to parse '{}'. Error: {}", uri, e.what());
                return nullptr;
            }

            AssetCpuShaderDefinition def;

            def.name = data["name"].as<std::string>();

            // Parse the SPIR-V file paths
            if (data["runtime"]) {
                for(const auto& node : data["runtime"]) {
                    auto stage_str = node.first.as<std::string>();
                    // You'll need a helper to convert "vertex" -> RAL::ShaderStage::Vertex, etc.
                    def.base_spirv_paths[string_to_shader_stage(stage_str)] = node.second.as<std::string>();
                }
            }

            // Parse features
            if (data["features"]) {
                for (const auto& node : data["features"]) {
                    def.features.push_back(node.as<std::string>());
                }
            }

            // Parse fixed pipeline state
            if (data["state"]) {
                const auto& state_node = data["state"];
                def.depth_test = state_node["depth_test"].as<bool>(true);
                def.depth_write = state_node["depth_write"].as<bool>(true);
                // ... etc. ...
            }

            // Parse vertex layout
            if (data["vertex_layout"]) {
                for (const auto& node : data["vertex_layout"]) {
                    VertexAttributeDesc attr;
                    attr.semantic_name = node["semantic"].as<std::string>();
                    // Helper to convert "RGB32F" -> RAL::Format::RGB32_SFLOAT
                    attr.format = string_to_ral_format(node["format"].as<std::string>());
                    def.vertex_layout.push_back(attr);
                }
            }

            // Populate the database
            auto& registry = db.get_registry();
            auto entity_id = registry.create();
            registry.emplace<AssetCpuShaderDefinition>(entity_id, std::move(def));
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, data["name"].as<std::string>());

            RDE_CORE_TRACE("ShaderDefLoader: Loaded definition for '{}'", data["name"].as<std::string>());
            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".shaderdef"};
        }
    };
}