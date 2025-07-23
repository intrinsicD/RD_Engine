#pragma once

#import "assets/ILoader.h"

namespace RDE{
    class MaterialManifestLoader : public ILoader {
    public:
        MaterialManifestLoader() = default;

        std::vector<std::string> get_dependencies(const std::string &uri) const override;

        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override;

        std::vector<std::string> get_supported_extensions() const override;
    };
}