#pragma once

#include "ILoader.h"
#include "AssetDatabase.h"
#include "AssetManager.h"

namespace RDE {
    class MeshMtlLoader : public ILoader {
    public:
        MeshMtlLoader() = default;

        std::vector<std::string> get_dependencies(const std::string &uri) const override;

        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override;

        std::vector<std::string> get_supported_extensions() const override;
    };
}