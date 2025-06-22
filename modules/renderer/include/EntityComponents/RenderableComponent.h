// file: modules/core/include/EntityComponents/RenderableComponent.h
#pragma once

#include "AssetHandle.h"

namespace RDE {
    struct RenderableComponent {
        AssetHandle mesh_handle = INVALID_ASSET_ID;
        AssetHandle material_handle = INVALID_ASSET_ID;
    };
}