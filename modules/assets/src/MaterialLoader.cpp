// In modules/core/src/Assets/AssetManager.cpp
#include "AssetManager.h"
#include "MaterialAsset.h"
#include "Log.h"
#include "FileIO.h"
#include "YamlUtils.h"

#include <string>

namespace RDE {
    // --- Material Specialization (YAML Version) ---
    template<>
    AssetHandle AssetManager::load<MaterialAsset>(const std::filesystem::path &path) {
        if (asset_registry.contains(path)) {
            return asset_registry.at(path);
        }

        // 1. Load the YAML file from disk.
        // NOTE: yaml-cpp can load directly from a file path, but reading it
        // into a string first allows for a more abstract FileIO system later.
        std::string yaml_string = FileIO::read_file(path);
        if (yaml_string.empty()) {
            RDE_CORE_ERROR("Failed to read material file: {}", path.string());
            return INVALID_ASSET_ID;
        }

        YAML::Node data;
        try {
            data = YAML::Load(yaml_string);
        } catch (YAML::ParserException &e) {
            RDE_CORE_ERROR("Failed to parse material YAML: {} \n  Error: {}", path.string(), e.what());
            return INVALID_ASSET_ID;
        }

        auto material = std::make_shared<MaterialAsset>();

        // 2. Load the shader dependency from the "shader" node.
        if (data["shader"]) {
            std::string shader_path = data["shader"].as<std::string>();
            material->shader_handle = load<ShaderAsset>(shader_path); // Recursively load dependency
        } else {
            RDE_CORE_ERROR("Material file '{}' is missing required 'shader' field.", path.string());
            return INVALID_ASSET_ID;
        }

        // 3. Load parameters from the "parameters" map node.
        if (data["parameters"]) {
            YAML::Node params = data["parameters"];
            for (YAML::const_iterator it = params.begin(); it != params.end(); ++it) {
                std::string name = it->first.as<std::string>();
                YAML::Node value = it->second;

                if (value.IsScalar()) {
                    // Try to parse as float, then int, then string (texture path)
                    try {
                        material->parameters[name] = value.as<float>();
                        continue;
                    } catch (...) {
                    }
                    try {
                        material->parameters[name] = value.as<int>();
                        continue;
                    } catch (...) {
                    }
                    try {
                        AssetHandle tex_handle = load<TextureAsset>(value.as<std::string>());
                        material->parameters[name] = tex_handle;
                        continue;
                    } catch (...) {
                    }
                } else if (value.IsSequence()) {
                    // Parse as vec3 or vec4
                    if (value.size() == 3) {
                        material->parameters[name] = value.as<glm::vec3>();
                    } else if (value.size() == 4) {
                        material->parameters[name] = value.as<glm::vec4>();
                    }
                }
            }
        }

        // 4. Register and return handle.
        AssetHandle handle = NewID();
        asset_registry[path] = handle;
        assets[handle] = material;

        RDE_CORE_INFO("Material loaded: {}", path.string());
        return handle;
    }
}
