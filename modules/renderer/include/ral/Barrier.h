#pragma once

#include "Common.h"
#include <vector>

namespace RAL {
    // Defines the memory layout of a texture, optimizing it for certain operations.
    // This is a CRITICAL concept for Vulkan correctness and performance.
    enum class TextureLayout {
        // The layout is unknown or uninitialized. Cannot be used directly.
        UNDEFINED,

        // Optimal for general-purpose access like shader reads/writes, but not specialized.
        GENERAL,

        // Optimal for being used as a color attachment (a render target).
        COLOR_ATTACHMENT_OPTIMAL,

        // Optimal for being used as a depth/stencil attachment.
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL,

        // Read-only variant for depth/stencil, for sampling (e.g., shadow mapping).
        DEPTH_STENCIL_READ_ONLY_OPTIMAL,

        // Optimal for being read by a shader (sampled texture, input attachment).
        SHADER_READ_ONLY_OPTIMAL,

        // Optimal for being the source of a transfer (copy) operation.
        TRANSFER_SRC_OPTIMAL,

        // Optimal for being the destination of a transfer (copy) operation.
        TRANSFER_DST_OPTIMAL,

        // The layout used for presenting the final image to the screen.
        PRESENT_SRC,
    };

    // Defines a pipeline stage for synchronization.
    // These map closely to VkPipelineStageFlagBits.
    enum class PipelineStage {
        TOP_OF_PIPE,
        DRAW_INDIRECT,
        VERTEX_INPUT,
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        COLOR_ATTACHMENT_OUTPUT,
        COMPUTE_SHADER,
        TRANSFER, // For copy operations
        BOTTOM_OF_PIPE,
        // ... many others
    };

    // Defines a type of memory access.
    // Maps to VkAccessFlagBits.
    enum class AccessFlags : uint32_t {
        NONE = 0,
        INDIRECT_COMMAND_READ = 1 << 0,
        INDEX_READ = 1 << 1,
        UNIFORM_READ = 1 << 2,
        SHADER_READ = 1 << 3,
        SHADER_WRITE = 1 << 4,
        COLOR_ATTACHMENT_READ = 1 << 5,
        COLOR_ATTACHMENT_WRITE = 1 << 6,
        DEPTH_STENCIL_ATTACHMENT_WRITE = 1 << 7,
        TRANSFER_READ = 1 << 8,
        TRANSFER_WRITE = 1 << 9,
        // ...
    };

    ENABLE_ENUM_FLAG_OPERATORS(AccessFlags) // Enable | and & operators

    // Describes a memory transition for a specific buffer.
    struct BufferMemoryBarrier {
        BufferHandle handle;
        AccessFlags src_access_mask;
        AccessFlags dst_access_mask;
    };

    // Describes a memory transition and layout change for a specific texture.
    struct TextureMemoryBarrier {
        TextureHandle handle;
        AccessFlags src_access_mask;
        AccessFlags dst_access_mask;
        // In Vulkan, layout transitions are critical for correctness and performance.
        TextureLayout old_layout;
        TextureLayout new_layout;
    };

    // The complete barrier description submitted to the command buffer.
    struct BarrierInfo {
        PipelineStage src_stage_mask;
        PipelineStage dst_stage_mask;

        std::vector<BufferMemoryBarrier> buffer_barriers;
        std::vector<TextureMemoryBarrier> texture_barriers;
        // A global memory barrier can also exist for simpler cases.
    };
} // namespace RAL
