#pragma once

#include "IAsset.h"

#include <memory>

namespace RDE{
    class IAssetLoader {
    public:
        virtual ~IAssetLoader() = default;

        virtual std::shared_ptr<IAsset> load(const std::string &path) = 0;

        AssetType get_asset_type() const { return m_asset_type; }

        const std::string &get_extension() const { return m_extension; }

        std::shared_ptr<IAsset> operator()(const std::string &path) {
            return load(path);
        }
    private:
        AssetType m_asset_type; // The type of asset this loader handles, e.g., Texture, Mesh, etc.
        std::string m_extension; // The file extension this loader handles, e.g., ".png", ".obj", etc.
    };
}