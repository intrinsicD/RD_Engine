// file: modules/assets/include/Assets/MaterialAsset.h
#pragma once

#include "IAsset.h"
#include "AssetHandle.h"

#include <glm/glm.hpp>
#include <variant>
#include <string>
#include <unordered_map>

namespace RDE {

    // A MaterialParameter can hold different types of data.
    // We use std::variant for a type-safe union. AssetHandle is used for textures.
    using MaterialParameter = std::variant<
        float,
        int,
        glm::vec3,
        glm::vec4,
        AssetHandle
    >;

    struct MaterialAsset : public IAsset {
        // --- DATA POPULATED BY ASSETMANAGER ---
        AssetHandle shader_handle = INVALID_ASSET_ID;
        std::unordered_map<std::string, MaterialParameter> parameters;
    };

} // namespace RDE