// assets/loaders/ShaderLoader.h
#pragma once

#include "core/Log.h"
#include "assets/ILoader.h"
#include "assets/AssetComponentTypes.h"

#include <fstream>
#include <sstream>

namespace RDE {
    class ShaderLoader final : public ILoader {
    public:
        AssetID load(const std::string& uri, AssetDatabase& db) const override {
            RDE_CORE_TRACE("ShaderLoader: Loading shader source from '{}'", uri);
            std::ifstream file(uri);
            if (!file.is_open()) {
                RDE_CORE_ERROR("ShaderLoader: Failed to open file '{}'", uri);
                return nullptr;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            auto& registry = db.get_registry();
            auto entity_id = registry.create();

            registry.emplace<AssetTextSource>(entity_id, buffer.str());
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, std::filesystem::path(uri).filename().string());

            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        std::vector<std::string> get_supported_extensions() const override {
            return {".glsl", ".vert", ".frag", ".comp"};
        }
    private:

    };
}