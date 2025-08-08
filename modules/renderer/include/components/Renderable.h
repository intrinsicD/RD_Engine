#pragma once
#include "ral/Common.h"

#include <vector>

namespace RDE {
    struct RenderableComponent {
        std::vector<RAL::BufferHandle> vertex_buffers; // Handles to vertex buffers
        RAL::BufferHandle index_buffer;

        uint32_t count;
        RAL::IndexType index_type = RAL::IndexType::UINT32; // Default to 32-bit indices
    };
}
