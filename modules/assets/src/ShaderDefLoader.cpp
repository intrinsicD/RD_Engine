// file: assets/ShaderDefLoader.cpp
#include "assets/ShaderDefLoader.h"
#include "assets/AssetComponentTypes.h"
#include "assets/AssetManager.h"
#include "core/Log.h"
#include "ral/EnumUtils.h"

#include <yaml-cpp/yaml.h>

namespace RDE {
    std::vector<std::string> ShaderDefLoader::get_supported_extensions() const {
        return {".shaderdef"};
    }

    // This loader parses the shader contract and stores it in an AssetShaderDef component.
    AssetID ShaderDefLoader::load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const {
        YAML::Node doc;
        try {
            doc = YAML::LoadFile(uri);
        } catch (const YAML::Exception &e) {
            RDE_CORE_ERROR("Failed to load/parse shader manifest '{}': {}", uri, e.what());
            return nullptr;
        }

        AssetShaderDef shaderDefComponent;

        if (doc["name"]) shaderDefComponent.name = doc["name"].as<std::string>();

        // --- Parse Dependencies ---
        if (const auto &depsNode = doc["dependencies"]) {
            for (YAML::const_iterator it = depsNode.begin(); it != depsNode.end(); ++it) {
                const auto key = it->first.as<std::string>();
                for (const auto &valNode: it->second) {
                    shaderDefComponent.dependencies[key].push_back(valNode.as<std::string>());
                }
            }
        }

        // --- Parse Features ---
        if (const auto &featuresNode = doc["features"]) {
            for (const auto &featureNode: featuresNode) {
                shaderDefComponent.features.push_back(featureNode.as<std::string>());
            }
        }

        // --- Parse Interface (The Contract) ---
        if (const auto &interfaceNode = doc["interface"]) {
            // Parse Vertex Attributes
            if (const auto &attrsNode = interfaceNode["vertex_attributes"]) {
                for (const auto &attrNode: attrsNode) {
                    RAL::VertexInputAttribute attr;
                    attr.location = attrNode["location"].as<uint32_t>();
                    attr.format = string_to_ral_format(attrNode["format"].as<std::string>());
                    attr.name = attrNode["semantic"].as<std::string>();
                    shaderDefComponent.vertexAttributes.emplace_back(attr);
                }
            }
            // Parse Descriptor Sets
            if (const auto &setsNode = interfaceNode["sets"]) {
                for (const auto &setNode: setsNode) {
                    RAL::DescriptorSetLayoutDescription setLayoutDesc;
                    setLayoutDesc.set = setNode["set"].as<uint32_t>();
                    for (const auto &bindingNode: setNode["bindings"]) {
                        RAL::DescriptorSetLayoutBinding binding;
                        binding.stages = string_to_shader_stages_mask(bindingNode["stage"].as<std::string>());
                        binding.binding = bindingNode["binding"].as<uint32_t>();
                        binding.type = string_to_descriptor_type(bindingNode["type"].as<std::string>());
                        binding.name = bindingNode["name"].as<std::string>();
                        setLayoutDesc.bindings.emplace_back(binding);
                    }
                    shaderDefComponent.descriptorSetLayouts.push_back(setLayoutDesc);
                }
            }
            // Parse Push Constants
            if (const auto &pushConstantsNode = interfaceNode["push_constants"]) {
                for (const auto &pcNode: pushConstantsNode) {
                    RAL::PushConstantRange pcRange;
                    pcRange.size = pcNode["size"].as<uint32_t>();
                    pcRange.stages = string_to_shader_stage(pcNode["stage"].as<std::string>());
                    pcRange.name = pcNode["name"].as<std::string>();
                    shaderDefComponent.pushConstantRanges.emplace_back(pcRange);
                }
            }
        }

        // --- Create the asset entity ---
        auto &registry = db.get_registry();
        entt::entity entity_id = registry.create();

        registry.emplace<AssetShaderDef>(entity_id, std::move(shaderDefComponent));
        registry.emplace<AssetName>(entity_id, shaderDefComponent.name);
        registry.emplace<AssetFilepath>(entity_id, uri);

        RDE_CORE_INFO("Loaded shader definition '{}'", uri);
        return std::make_shared<AssetID_Data>(entity_id, uri);
    }

    // Fast dependency scan of the YAML file.
    std::vector<std::string> ShaderDefLoader::get_dependencies(const std::string &uri) const {
        std::vector<std::string> deps;
        YAML::Node doc;
        try {
            doc = YAML::LoadFile(uri);
        } catch (const YAML::Exception &e) { return deps; }

        if (const auto &depsNode = doc["dependencies"]) {
            for (YAML::const_iterator it = depsNode.begin(); it != depsNode.end(); ++it) {
                for (const auto &valNode: it->second) {
                    deps.push_back(valNode.as<std::string>());
                }
            }
        }
        return deps;
    }
}