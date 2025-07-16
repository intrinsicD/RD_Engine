#pragma once

#include "assets/ILoader.h"
#include "assets/AssetManager.h"
#include "assets/AssetComponentTypes.h" // We'll add the new components here
#include "assets/AssetConcepts.h"
#include "core/Paths.h"
#include <yaml-cpp/yaml.h>

namespace RDE {
    class ProgramLoader final : public ILoader {
    public:
        explicit ProgramLoader(AssetManager *asset_manager) : m_asset_manager(asset_manager) {}

        AssetID load(const std::string &uri, AssetDatabase &db) const override {
            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception &e) {
                RDE_CORE_ERROR("ProgramLoader: Failed to load/parse YAML file '{}'. What: {}", uri, e.what());
                return nullptr;
            }

            // --- 1. Load all referenced SPIR-V modules as their own assets ---
            AssetCpuShaderProgram program_stages;
            auto spirv_base_path = get_asset_path().value() / "shaders" / "spirv"; // Relative path within assets

            if (data["runtime"]) {
                const auto &runtime = data["runtime"];
                if (runtime["vertex"]) {
                    std::string spirv_filename = runtime["vertex"].as<std::string>();
                    program_stages.vertex_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
                if (runtime["fragment"]) {
                    std::string spirv_filename = runtime["fragment"].as<std::string>();
                    program_stages.fragment_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
                if (runtime["geometry"]) {
                    std::string spirv_filename = runtime["geometry"].as<std::string>();
                    program_stages.geometry_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
                if (runtime["tessellation_control"]) {
                    std::string spirv_filename = runtime["tessellation_control"].as<std::string>();
                    program_stages.tessellation_control_shader = m_asset_manager->load(
                            spirv_base_path / spirv_filename);
                }
                if (runtime["tessellation_evaluation"]) {
                    std::string spirv_filename = runtime["tessellation_evaluation"].as<std::string>();
                    program_stages.tessellation_evaluation_shader = m_asset_manager->load(
                            spirv_base_path / spirv_filename);
                }
                if (runtime["compute"]) {
                    std::string spirv_filename = runtime["compute"].as<std::string>();
                    program_stages.compute_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
                if (runtime["task"]) {
                    std::string spirv_filename = runtime["task"].as<std::string>();
                    program_stages.task_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
                if (runtime["mesh"]) {
                    std::string spirv_filename = runtime["mesh"].as<std::string>();
                    program_stages.mesh_shader = m_asset_manager->load(spirv_base_path / spirv_filename);
                }
            }

            if (!program_stages.vertex_shader &&
                !program_stages.fragment_shader &&
                !program_stages.geometry_shader &&
                !program_stages.tessellation_control_shader &&
                !program_stages.tessellation_evaluation_shader &&
                !program_stages.compute_shader &&
                !program_stages.task_shader &&
                !program_stages.mesh_shader) {
                RDE_CORE_ERROR("ProgramLoader: Program '{}' defines no valid runtime shader stages.", uri);
                return nullptr;
            }

            // --- 2. Create the main ShaderProgram asset that links them ---
            auto &registry = db.get_registry();
            auto entity_id = registry.create();

            // Emplace the component containing the AssetIDs of the stages
            registry.emplace<AssetCpuShaderProgram>(entity_id, program_stages);

            // Add metadata
            std::string name = data["name"] ? data["name"].as<std::string>() : std::filesystem::path(
                    uri).stem().string();
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, name);

            RDE_CORE_TRACE("ProgramLoader: Successfully loaded program '{}' linking its stages.", name);

            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".program"};
        }

    private:
        AssetManager *m_asset_manager;
    };
}