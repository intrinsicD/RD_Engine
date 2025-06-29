#pragma once

#include "Common.h" // For handles

namespace RAL {
    // Defines how a resource will be used. Critical for optimization.
    enum class ResourceUsage {
        STATIC,  // Written once, read many times (e.g., static vertex buffer)
        DYNAMIC, // Updated frequently from the CPU (e.g., dynamic uniform buffer)
        GPU_ONLY // Only ever written to and read from by the GPU (e.g., G-Buffer targets)
    };

    struct BufferDescription {
        uint64_t size = 0;
        ResourceUsage usage = ResourceUsage::STATIC;
        // Could also have flags for vertex, index, uniform buffer types
    };
    
    struct TextureDescription {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1; // For 3D textures
        uint32_t mip_levels = 1;
        Format format = Format::UNKNOWN; // You need a Format enum in Common.h
        ResourceUsage usage = ResourceUsage::STATIC;
    };
}