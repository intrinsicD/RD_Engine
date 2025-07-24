#pragma once

#include "assets/AssetHandle.h"
#include "assets/AssetComponentTypes.h"
#include "ral/Resources.h"

namespace RDE {
    struct GeometryComponent {
        AssetID geometry_asset_id;
    };

    struct RenderableGeometryComponent {
        std::unordered_map<std::string, RAL::BufferHandle> attribute_buffers;
        RAL::BufferHandle index_buffer;

        // We copy the subview info for direct access.
        std::vector<AssetGeometrySubView> subviews;
        uint32_t index_count;
        RAL::IndexType index_type;
    };
}
