// assets/loaders/ShaderDefLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"
#include "ral/EnumUtils.h"

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

            if(data["version"] && !check_version(data["version"].as<int>())) {
                RDE_CORE_ERROR("ShaderDefLoader: Unsupported shader definition version in '{}'. Expected 1, got {}",
                               uri, data["version"].as<int>());
                return nullptr;
            }

            //read dependencies
            //check if the compiled shader stages exist
            //if not compile them and create them
            //if yes, just load them and create the pipeline

            if(data["dependencies"]) {
                const auto &dependencies = data["dependencies"];
                if(dependencies["sources"]) {

                }

            }



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