#pragma once

#include "assets/AssetHandle.h"

namespace RDE{
    struct MaterialComponent{
        AssetID material_asset_id; // Reference to the material asset

        PropertyContainer parameters;

        std::unordered_map<std::string, AssetID> texture_bindings;

        bool is_valid() const {
            return material_asset_id ? material_asset_id->is_valid() : false;
        }
    };
}