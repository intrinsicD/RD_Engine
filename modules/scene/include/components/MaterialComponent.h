#pragma once

#include "assets/AssetHandle.h"
#include "ral/Common.h"

#include <variant>
#include <unordered_map>

namespace RDE{
    struct SlotSource{
        std::string slot_name; // Name of the slot in the material
        std::variant<AssetID, RAL::BufferHandle, RAL::TextureHandle> source; // Can be either an AssetID or a BufferHandle
    };
    struct MaterialComponent {
        AssetID material_asset_id; // Reference to the material asset in the AssetDatabase

        std::unordered_map<std::string, SlotSource> slot_sources; // Maps slot names to their sources
        SlotSource vertex_color_source; // Special slot for vertex color, if applicable
        // You can add more fields here as needed, like shader handles, properties, etc.
    };
}