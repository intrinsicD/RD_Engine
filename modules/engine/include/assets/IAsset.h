#pragma once

#include "assets/AssetID.h"
#include "IRenderer.h"

namespace RDE {
    enum class AssetType {
        None = 0,
        Scene,
        Texture,
        Geometry,
        Material,
        Shader,
        // Add more asset types as needed
    };

    class IAsset {
    public:
        virtual ~IAsset() = default;

        AssetID ID = INVALID_ASSET_ID;
        AssetType type = AssetType::None;
    };
}
