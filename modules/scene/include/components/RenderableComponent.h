#pragma once

#include "assets/AssetHandle.h"

namespace RDE{
    struct RenderableComponent{
        AssetID geometry_id; // Reference to the asset in the AssetDatabase

        bool isVisible = true;

        bool is_valid() const {
            return geometry_id ? geometry_id->is_valid() : false;
        }
    };
}