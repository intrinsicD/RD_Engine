#pragma once

#include <cstdint>

namespace RDE{
    using AssetID = uint32_t;
    const AssetID INVALID_ASSET_ID = 0;

    enum class AssetType {
        None = 0,
        Scene,
        Texture,
        Geometry,
        Material,
        Shader,
        // Add more asset types as needed
    };

    // spdlog support for AssetType output TODO

}