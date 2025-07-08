//ral/Common.h
#pragma once

#include "core/EnumFlags.h" // For ENABLE_ENUM_FLAG_OPERATORS

#include <cstdint>
#include <limits>

namespace RAL {
    struct RenderHandle {
        uint32_t index{std::numeric_limits<uint32_t>::max()};
        uint32_t generation{0};

        // Define an explicit invalid state.
        static constexpr RenderHandle INVALID() { return {}; }

        constexpr bool is_valid() const {
            return index != std::numeric_limits<uint32_t>::max();
        }

        // For use in std::map, etc.
        bool operator<(const RenderHandle &other) const {
            if (index < other.index) return true;
            if (index > other.index) return false;
            return generation < other.generation;
        }

        bool operator==(const RenderHandle &other) const {
            return index == other.index && generation == other.generation;
        }
    };

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


    enum class Format {
        UNKNOWN,

        // --- 8-bit formats ---
        R8_UNORM,           // Single channel, unsigned normalized
        R8G8_UNORM,         // Two channels
        R8G8B8A8_UNORM,     // Four channels
        B8G8R8A8_UNORM,     // Common for swapchains

        R8_SRGB,            // Single channel, sRGB gamma corrected
        R8G8_SRGB,          // Two channels
        R8G8B8A8_SRGB,      // Four channels
        B8G8R8A8_SRGB,      // Common for swapchains with sRGB

        // --- 16-bit formats ---
        R16_SFLOAT,         // Half-precision float
        R16G16_SFLOAT,
        R16G16B16A16_SFLOAT,

        // --- 32-bit formats (very common for vertex attributes) ---
        R32_SFLOAT,         // Single-precision float
        R32G32_SFLOAT,      // vec2
        R32G32B32_SFLOAT,   // vec3
        R32G32B32A32_SFLOAT,// vec4

        R32_UINT,           // Unsigned integer
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,

        // --- Depth/Stencil Formats ---
        D32_SFLOAT,         // 32-bit float depth buffer
        D24_UNORM_S8_UINT,  // Common 24-bit depth, 8-bit stencil format
        D32_SFLOAT_S8_UINT, // 32-bit float depth, 8-bit stencil

        // --- Block Compression Formats (for textures) ---
        BC1_RGB_UNORM,      // DXT1
        BC3_UNORM,          // DXT5
        BC7_UNORM,          // High quality compression
    };
}
