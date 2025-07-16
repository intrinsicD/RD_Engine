// assets/loaders/MaterialLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetManager.h"
#include "core/Paths.h"

#include "ral/Resources.h"

#include <yaml-cpp/yaml.h> // The YAML-CPP header
#include <fstream>

namespace RDE {
    class MaterialLoader final : public ILoader {
    public:
        explicit MaterialLoader(AssetManager *asset_manager)
            : m_asset_manager(asset_manager) {

        }

        AssetID load(const std::string &uri, AssetDatabase &db) const override {
            YAML::Node data;
            try {
                data = YAML::LoadFile(uri);
            } catch (const YAML::Exception &e) {
                RDE_CORE_ERROR("MaterialLoader: Failed to load/parse YAML file '{}'. What: {}", uri, e.what());
                return nullptr;
            }

            AssetCpuMaterial cpu_mat;

            if (data["name"]) {
                cpu_mat.name = data["name"].as<std::string>();
            }
            if (data["shader"]) {
                cpu_mat.shader_path = data["shader"].as<std::string>();
            }

            if (data["pipeline"]) {
                const auto &pipeline_node = data["pipeline"];
                // You would have a map or helper for string-to-enum conversion here.
                // For now, we'll keep it simple.
                auto cullModeStr = pipeline_node["cullMode"].as<std::string>("Back");
                if (cullModeStr == "None") cpu_mat.cull_mode = RAL::CullMode::None;
                else if (cullModeStr == "Front") cpu_mat.cull_mode = RAL::CullMode::Front;
                else cpu_mat.cull_mode = RAL::CullMode::Back;

                cpu_mat.depth_test = pipeline_node["depthTest"].as<bool>(true);
                cpu_mat.depth_write = pipeline_node["depthWrite"].as<bool>(true);
            }

            if (data["parameters"]) {
                const auto &params_node = data["parameters"];
                if (params_node["baseColor"]) {
                    cpu_mat.vector_params["baseColor"] = params_node["baseColor"].as<glm::vec4>();
                }
                cpu_mat.float_params["metalness"] = params_node["metalness"].as<float>(0.0f);
                cpu_mat.float_params["roughness"] = params_node["roughness"].as<float>(1.0f);
            }

            if (data["textures"]) {
                auto base_path = get_asset_path().value();
                for (const auto &texture_node: data["textures"]) {
                    auto sampler_name = texture_node.first.as<std::string>();
                    auto texture_path = texture_node.second.as<std::string>();

                    std::string full_texture_path = (base_path / texture_path).string();

                    if (AssetID texture_id = m_asset_manager->load(full_texture_path)) {
                        cpu_mat.texture_asset_ids[sampler_name] = texture_id;
                    }
                }
            }

            // The rest of the function (populating database, returning receipt) is identical.
            auto &registry = db.get_registry();
            auto entity_id = registry.create();

            auto name = cpu_mat.name;
            registry.emplace<AssetCpuMaterial>(entity_id, std::move(cpu_mat));
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, name);

            RDE_CORE_TRACE("MaterialLoader: Successfully loaded '{}'", name);

            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".mat"}; // Support YAML extensions
        }

    private:
        AssetManager *m_asset_manager;
        std::vector<std::string> m_supported_extensions;
    };
}

// NOTE: You will likely need to implement a `YAML::convert` specialization for glm::vec4
// so that `node.as<glm::vec4>()` works automatically.
namespace YAML {
    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node &node, glm::vec4 &rhs) {
            if (!node.IsSequence() || node.size() != 4) {
                return false;
            }
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}
