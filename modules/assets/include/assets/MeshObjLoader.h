#pragma once

#include "ILoader.h"

namespace RDE {
    class MeshObjLoader : public ILoader {
    public:
        MeshObjLoader() = default;

        // Fast dependency discovery for OBJ files
        std::vector<std::string> get_dependencies(const std::string &uri) const override;

        AssetID load_asset(const std::string &uri, AssetDatabase &db, AssetManager &manager) const override;

        std::vector<std::string> get_supported_extensions() const override;

    };
}