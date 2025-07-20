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
        std::vector<std::string> get_dependencies(const std::string &uri) const override {
            (void) uri;
            return {};
        }

        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override {
            (void) manager; // No other assets to look up.

            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception &e) {
                RDE_CORE_ERROR("ShaderDefLoader: Failed to parse '{}'. Error: {}", uri, e.what());
                return nullptr;
            }

            if (data["version"] && !check_version(data["version"].as<int>())) {
                RDE_CORE_ERROR("ShaderDefLoader: Unsupported shader definition version in '{}'. Expected 1, got {}",
                               uri, data["version"].as<int>());
                return nullptr;
            }

            if (!data["name"]) {
                RDE_CORE_ERROR("ShaderDefLoader: Missing shader definition name.");
                return nullptr;
            }
            auto shaderdef_name = data["name"].as<std::string>();

            //read dependencies
            //check if the compiled shader stages exist
            //if not compile them and create them
            //if yes, just load them and create the pipeline

            if (!data["dependencies"]) {
                RDE_CORE_ERROR("ShaderDefLoader: Missing dependencies.");
                return nullptr;
            }
            std::vector<std::string> dependencies_sources;
            std::vector<std::string> dependencies_spirv;

            const auto dep = data["dependencies"];

            if (dep["source"]) {
                for (const auto &source: dep["source"]) {
                    dependencies_sources.push_back(dep["source"].as<std::string>());
                }
            }
            if (dep["spirv"]) {
                for (const auto &spirv: dep["spirv"]) {
                    dependencies_spirv.push_back(dep["spirv"].as<std::string>());
                }
            }

            if (!data["program"]) {
                RDE_CORE_ERROR("ShaderDefLoader: Missing shader program definition.");
                return nullptr;
            }

            const auto program = data["program"];
            int vertex_shader = program["vertex"].as<int>();
            int fragment_shader = program["fragment"].as<int>();
            int compute = program["compute"].as<int>(-1);
            //TODO add all other stages

            if (dependencies_spirv.size() < 2 && compute == -1) {
                RDE_CORE_ERROR("ShaderDefLoader: Not enough shader stages defined in '{}'. At least vertex and fragment or compute are required.", uri);
                return nullptr;
            }

            //TODO unsure how to continue from here.
            AssetCpuShaderDefinition def;
            def.name = shaderdef_name;
            def.base_spirv_paths = {
                {RAL::ShaderStage::Vertex, dependencies_spirv[vertex_shader]},
                {RAL::ShaderStage::Fragment, dependencies_spirv[fragment_shader]}
            };
            if (compute != -1) {
                def.base_spirv_paths[RAL::ShaderStage::Compute] = dependencies_spirv[compute];
            }
            def.dependencies = dependencies_sources; // Assuming these are the source files needed for compilation
            // Add other stages as needed, e.g., geometry, tessellation, etc.
            // def.base_spirv_paths[RAL::ShaderStage::Geometry] = dependencies_spirv[geometry_shader];
            // def.base_spirv_paths[RAL::ShaderStage::TessellationControl] = dependencies_spirv[tess_control_shader];
            // def.base_spirv_paths[RAL::ShaderStage::TessellationEvaluation] = dependencies_spirv[tess_eval_shader];
            // def.base_spirv_paths[RAL::ShaderStage::RayTracing] = dependencies_spirv[ray_tracing_shader];
            // def.base_spirv_paths[RAL::ShaderStage::Task] = dependencies_spirv[task_shader];
            // def.base_spirv_paths[RAL::ShaderStage::Mesh] = dependencies_spirv[mesh_shader];

            if (data["state"]) {
                RDE_CORE_WARN("TODO! actually read the data and map it!");
                def.state.add<RAL::CullMode>("cull_mode", RAL::CullMode::Back); // Default to Back
                def.state.add<bool>("depth_test", true);
                def.state.add<bool>("depth_write", true);
                // Add more state properties as needed
            }




            auto &registry = db.get_registry();
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
