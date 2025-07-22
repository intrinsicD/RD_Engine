// assets/loaders/StbImageLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetManager.h"

namespace RDE {
    class StbImageLoader final : public ILoader {
    public:
        StbImageLoader() = default;

        std::vector<std::string> get_dependencies(const std::string &uri) const override;

        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override;

        std::vector<std::string> get_supported_extensions() const override;
    };
} // namespace RDE```
