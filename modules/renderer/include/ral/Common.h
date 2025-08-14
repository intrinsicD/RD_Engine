//ral/Common.h
#pragma once

#include "core/EnumFlags.h" // For ENABLE_ENUM_FLAG_OPERATORS
#include "RenderHandle.h"

#include <cstdint>
#include <limits>

namespace RAL {
    struct BufferHandle : RenderHandle {
        static constexpr BufferHandle INVALID() { return {}; }
    };

    struct TextureHandle : RenderHandle {
        static constexpr TextureHandle INVALID() { return {}; }
    };

    struct SamplerHandle : RenderHandle {
        static constexpr SamplerHandle INVALID() { return {}; }
    };

    struct PipelineHandle : RenderHandle {
        static constexpr PipelineHandle INVALID() { return {}; }
    };

    struct ShaderHandle : RenderHandle {
        static constexpr ShaderHandle INVALID() { return {}; }
    };

    struct DescriptorSetLayoutHandle : RenderHandle {
        static constexpr DescriptorSetLayoutHandle INVALID() { return {}; }
    };

    struct DescriptorSetHandle : RenderHandle {
        static constexpr DescriptorSetHandle INVALID() { return {}; }
    };


    template<typename T>
    struct ResourceSlot {
        T resource; // The actual Vulkan handle (e.g., VkBuffer, VkImage)
        uint32_t generation{0};
    };

    enum class IndexType {
        UINT16, UINT32
    };

    struct Viewport {
        float x, y, width, height, min_depth, max_depth;
    };

    struct Rect2D {
        int32_t x, y;
        uint32_t width, height;
    };

    struct Offset3D {
        int32_t x, y, z;
    };

    struct Extent3D {
        uint32_t width, height, depth;
    };

    enum class Format {
        UNKNOWN,

        // --- 8-bit formats ---
        R8_UNORM, // Single channel, unsigned normalized
        R8G8_UNORM, // Two channels
        R8G8B8A8_UNORM, // Four channels
        B8G8R8A8_UNORM, // Common for swapchains

        R8_SRGB, // Single channel, sRGB gamma corrected
        R8G8_SRGB, // Two channels
        R8G8B8A8_SRGB, // Four channels
        B8G8R8A8_SRGB, // Common for swapchains with sRGB

        // --- 16-bit formats ---
        R16_SFLOAT, // Half-precision float
        R16G16_SFLOAT,
        R16G16B16A16_SFLOAT,

        // --- 32-bit formats (very common for vertex attributes) ---
        R32_SFLOAT, // Single-precision float
        R32G32_SFLOAT, // vec2
        R32G32B32_SFLOAT, // vec3
        R32G32B32A32_SFLOAT, // vec4

        R32_UINT, // Unsigned integer
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,

        // --- Depth/Stencil Formats ---
        D32_SFLOAT, // 32-bit float depth buffer
        D24_UNORM_S8_UINT, // Common 24-bit depth, 8-bit stencil format
        D32_SFLOAT_S8_UINT, // 32-bit float depth, 8-bit stencil

        // --- Block Compression Formats (for textures) ---
        BC1_RGB_UNORM, // DXT1
        BC3_UNORM, // DXT5
        BC7_UNORM, // High quality compression
    };

    inline uint32_t get_size_of_format(RAL::Format format) {
        switch (format) {
            // 8-bit
            case RAL::Format::R8_UNORM: return 1;
            case RAL::Format::R8G8_UNORM: return 2;
            case RAL::Format::R8G8B8A8_UNORM:
            case RAL::Format::B8G8R8A8_UNORM:
            case RAL::Format::R8_SRGB: // sRGB affects interpretation, not size
            case RAL::Format::R8G8_SRGB:
            case RAL::Format::R8G8B8A8_SRGB:
            case RAL::Format::B8G8R8A8_SRGB: return 4;

                // 16-bit
            case RAL::Format::R16_SFLOAT: return 2;
            case RAL::Format::R16G16_SFLOAT: return 4;
            case RAL::Format::R16G16B16A16_SFLOAT: return 8;

                // 32-bit
            case RAL::Format::R32_SFLOAT:
            case RAL::Format::R32_UINT: return 4;

            case RAL::Format::R32G32_SFLOAT:
            case RAL::Format::R32G32_UINT: return 8;

            case RAL::Format::R32G32B32_SFLOAT:
            case RAL::Format::R32G32B32_UINT: return 12;

            case RAL::Format::R32G32B32A32_SFLOAT:
            case RAL::Format::R32G32B32A32_UINT: return 16;

                // Depth/Stencil - we usually care about the depth part for size.
            case RAL::Format::D32_SFLOAT: return 4;
            case RAL::Format::D24_UNORM_S8_UINT: return 4; // 32 bits total
            case RAL::Format::D32_SFLOAT_S8_UINT: return 5; // Not standard, but for completeness

            default: return 0; // Unknown or block-compressed formats
        }
    }
}
