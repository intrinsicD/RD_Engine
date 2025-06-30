#pragma once

#include "EnumFlags.h" // For ENABLE_ENUM_FLAG_OPERATORS

#include <cstdint>

namespace RAL {
    // All handles are simple, type-safe wrappers around an ID or pointer.
#define RAL_HANDLE(Name) struct Name { uint64_t id = 0; bool is_valid() const { return id != 0; } };

    RAL_HANDLE(PipelineHandle);

    RAL_HANDLE(BufferHandle);

    RAL_HANDLE(TextureHandle);

    RAL_HANDLE(DescriptorSetLayoutHandle);

    RAL_HANDLE(DescriptorSetHandle);

    RAL_HANDLE(SemaphoreHandle);

    // Defines the data format for texture and buffer elements.
    // The names are based on the Vulkan naming convention for clarity and ease of mapping.
    enum class Format {
        UNKNOWN,

        // --- Color Formats (Unsigned Normalized) ---
        R8_UNORM,
        R8G8_UNORM,
        R8G8B8_UNORM,
        R8G8B8A8_UNORM,

        B8G8R8_UNORM,
        B8G8R8A8_UNORM,

        R16_UNORM,
        R16G16_UNORM,
        R16G16B16_UNORM,
        R16G16B16A16_UNORM,

        // --- Color Formats (sRGB - Gamma Corrected) ---
        R8_SRGB,
        R8G8_SRGB,
        R8G8B8_SRGB,
        R8G8B8A8_SRGB,

        B8G8R8_SRGB,
        B8G8R8A8_SRGB,

        // --- Color Formats (Signed Floating Point) ---
        R16_SFLOAT,
        R16G16_SFLOAT,
        R16G16B16_SFLOAT,
        R16G16B16A16_SFLOAT,

        R32_SFLOAT,
        R32G32_SFLOAT,
        R32G32B32_SFLOAT,
        R32G32B32A32_SFLOAT,

        // --- Color Formats (Integer) ---
        R32_UINT,
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,

        // --- Depth/Stencil Formats ---
        D16_UNORM,
        D32_SFLOAT,
        D24_UNORM_S8_UINT, // 24 bits for depth (normalized), 8 bits for stencil (integer)
        D32_SFLOAT_S8_UINT,

        // --- Block Compression Formats (very common for textures) ---
        BC1_RGB_UNORM,   // DXT1
        BC1_RGBA_UNORM,  // DXT1 with 1-bit alpha
        BC2_UNORM,       // DXT3
        BC3_UNORM,       // DXT5
        BC4_UNORM,       // For single-channel data (e.g., roughness map)
        BC5_UNORM,       // For two-channel data (e.g., normal map)
        BC7_UNORM,       // High quality compression

        // Add other formats as needed (e.g., HDR formats like B10G11R11_UFLOAT_PACK32)
    };

    enum class IndexType { UINT16, UINT32 };

    struct Viewport {
        float x, y, width, height, min_depth, max_depth;
    };

    struct Rect2D {
        int32_t x, y;
        uint32_t width, height;
    };

    struct SwapchainDescription {
        void* native_window_handle = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        bool vsync = true;
    };

    // etc.
}
