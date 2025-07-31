#include "assets/MaterialManifestLoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetManager.h"
#include "material/MaterialDescription.h"
#include "ral/EnumUtils.h"

#include <yaml-cpp/yaml.h>

namespace RDE {

    std::vector<std::string> MaterialManifestLoader::get_supported_extensions() const {
        return {".mat"}; // Supported extensions for material manifests
    }

    // This loader is responsible for reading the final .mat manifest
    AssetID MaterialManifestLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        YAML::Node doc;
        try {
            doc = YAML::LoadFile(uri);
        } catch (const YAML::Exception &e) {
            RDE_CORE_ERROR("Failed to load/parse material manifest '{}': {}", uri, e.what());
            return nullptr;
        }

        // --- Data objects we will populate ---
        MaterialDescription material;
        AssetPipelineDescription pipelineDesc;
        std::string assetName;

        // --- Version & Name Parsing ---
        if (doc["version"] && doc["version"].as<std::string>() != "1.0") {
            RDE_CORE_WARN("Material '{}' has unsupported version. Expected 1.0.", uri);
            // Decide if this should be a fatal error
        }

        if (doc["name"]) {
            assetName = doc["name"].as<std::string>();
        } else {
            assetName = std::filesystem::path(uri).stem().string();
        }
        material.name = assetName;

        // --- Dependency Linking ---
        if (doc["dependencies"] && doc["dependencies"]["shaders"]) {
            // A material should only depend on ONE shader definition
            const auto shader_def_path = doc["dependencies"]["shaders"][0].as<std::string>();
            material.pipeline = manager.get_loaded_asset(shader_def_path);
        } else {
            RDE_CORE_ERROR("Material '{}' is missing shader dependency.", uri);
            return nullptr;
        }

        // --- NEW: Pipeline State Parsing ---
        if (const auto &pipelineNode = doc["pipeline"]) {
            if (pipelineNode["cullMode"]) {
                // Here you would have a function to convert string to your RAL enum
                pipelineDesc.cullMode = string_to_cull_mode(pipelineNode["cullMode"].as<std::string>());
            }
            if (pipelineNode["polygonMode"]) {
                pipelineDesc.polygonMode = string_to_polygon_mode(pipelineNode["polygonMode"].as<std::string>());
            }
            if (pipelineNode["depthTest"]) {
                pipelineDesc.depthTest = pipelineNode["depthTest"].as<bool>();
            }
            if (pipelineNode["depthWrite"]) {
                pipelineDesc.depthWrite = pipelineNode["depthWrite"].as<bool>();
            }
        }

        // --- Corrected Parameter Parsing ---
        if (const auto &paramsNode = doc["parameters"]) {
            for (const auto &paramNode: paramsNode) {
                const auto param_name = paramNode["name"].as<std::string>();
                const auto param_type = paramNode["type"].as<std::string>();
                const auto &valueNode = paramNode["value"];

                if (param_type == "float") {
                    material.parameters.add<float>("p:" + param_name, valueNode.as<float>());
                } else if (param_type == "vec2") {
                    auto v = valueNode.as<std::vector<float>>();
                    material.parameters.add<glm::vec2>("p:" + param_name, {v[0], v[1]});
                } else if (param_type == "vec3") {
                    auto v = valueNode.as<std::vector<float>>();
                    material.parameters.add<glm::vec3>("p:" + param_name, {v[0], v[1], v[2]});
                } else if (param_type == "vec4") {
                    auto v = valueNode.as<std::vector<float>>();
                    material.parameters.add<glm::vec4>("p:" + param_name, {v[0], v[1], v[2], v[3]});
                } else {
                    RDE_CORE_WARN("Unsupported parameter type '{}' in '{}'", param_type, uri);
                }
            }
        }

        // --- Corrected Texture Linking ---
        if (doc["dependencies"]["textures"] && doc["textures"]) {
            const auto &texture_deps = doc["dependencies"]["textures"];
            for (const auto &textureNode: doc["textures"]) {
                const auto texture_name = textureNode["name"].as<std::string>();
                int idx = textureNode["index"].as<int>();

                if (idx < texture_deps.size()) {
                    const auto texture_uri = texture_deps[idx].as<std::string>();
                    material.textures["t_" + texture_name] = manager.get_loaded_asset(texture_uri);
                } else {
                    RDE_CORE_WARN("Texture index {} out of bounds for '{}' in '{}'", idx, texture_name, uri);
                }
            }
        }

        // --- Final Asset Creation ---
        auto &registry = db.get_registry();
        entt::entity entity_id = registry.create();

        registry.emplace<MaterialDescription>(entity_id, std::move(material));
        registry.emplace<AssetPipelineDescription>(entity_id, pipelineDesc); // Emplace the new component
        registry.emplace<AssetName>(entity_id, assetName);
        registry.emplace<AssetFilepath>(entity_id, uri);

        RDE_CORE_INFO("Loaded material manifest '{}'", uri);
        return std::make_shared<AssetID_Data>(entity_id, uri);
    }

    std::vector<std::string> MaterialManifestLoader::get_dependencies(const std::string &uri) const {
        // A fast dependency scan of the YAML file
        std::vector<std::string> deps;
        YAML::Node doc = YAML::LoadFile(uri);

        for (const auto &node: doc["dependencies"]["shaders"]) {
            deps.push_back(node.as<std::string>());
        }
        for (const auto &node: doc["dependencies"]["textures"]) {
            deps.push_back(node.as<std::string>());
        }
        return deps;
    }
}