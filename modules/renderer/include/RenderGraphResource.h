#pragma once

#include <cstdint>

namespace RDE {
    // Represents a "virtual" resource within the graph.
    // It doesn't correspond to a real GpuTextureHandle until the graph is executed.
    // The index points to the RenderGraph's internal resource list.
    struct RGResourceHandle {
        uint32_t index = -1;
        bool is_valid() const { return index != -1; }
    };
}
