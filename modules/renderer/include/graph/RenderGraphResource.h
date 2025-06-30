#pragma once

#include <cstdint>

namespace RDE {
    // Represents a "virtual" resource within the graph.
    // It doesn't correspond to a real GpuTextureHandle until the graph is executed.
    // The index points to the RenderGraph's internal resource list.

    struct RGBufferDescription{};
    struct RGTextureDescription{};


    template <typename T>
    struct RGResourceHandle {
        uint32_t index = -1;

        bool is_valid() const { return index != -1; }

        bool operator==(const RGResourceHandle &other) const { return index == other.index; }

        bool operator!=(const RGResourceHandle &other) const { return index != other.index; }

        bool operator<(const RGResourceHandle &other) const { return index < other.index; } // For std::map
    };

    using RGBufferHandle = RGResourceHandle<RGBufferDescription>;
    using RGTextureHandle = RGResourceHandle<RGTextureDescription>;
}
