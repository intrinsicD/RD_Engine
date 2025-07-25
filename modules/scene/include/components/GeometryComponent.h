#pragma once

#include "assets/AssetHandle.h"
#include "assets/AssetComponentTypes.h"
#include "ral/Resources.h"

namespace RDE {
    struct GeometryComponent {
        AssetID geometry_asset_id;
    };

    struct RenderableGeometryComponent {
        // Can hold one buffer (interleaved/planar) or many (separate attributes)
        std::vector<RAL::BufferHandle> vertex_buffers;

        // The full description of the vertex layout, to be used by the pipeline
        std::vector<RAL::VertexInputBinding> vertex_bindings;
        std::vector<RAL::VertexInputAttribute> vertex_attributes;

        // Index buffer remains the same
        RAL::BufferHandle index_buffer;
        uint32_t index_count = 0;
        RAL::IndexType index_type = RAL::IndexType::UINT32;

        uint32_t vertex_count = 0;
    };
}
