//ral/Device.h
#pragma once

#include "Common.h"
#include "CommandBuffer.h"

#include <memory>
#include <vector>

namespace RAL {
    // Forward declarations
    struct BufferDescription;
    struct TextureDescription;
    struct ShaderDescription;
    struct PipelineDescription;
    struct SwapchainDescription;
    struct DescriptorSetLayoutDescription;
    struct DescriptorSetDescription;

    class Device {
    public:
        virtual ~Device() = default;

        // --- NEW Swapchain Management ---
        virtual void create_swapchain(const SwapchainDescription &desc) = 0;

        virtual void destroy_swapchain() = 0;

        virtual void* map_buffer(BufferHandle handle) = 0;

        virtual void unmap_buffer(BufferHandle handle) = 0;

        virtual BufferHandle create_buffer(const BufferDescription &desc) = 0;

        virtual void destroy_buffer(BufferHandle handle) = 0;

        virtual TextureHandle create_texture(const TextureDescription &desc) = 0;

        virtual void destroy_texture(TextureHandle handle) = 0;

        virtual ShaderHandle create_shader(const ShaderDescription &desc) = 0;

        virtual void destroy_shader(ShaderHandle handle) = 0;

        virtual PipelineHandle create_pipeline(const PipelineDescription &desc) = 0;

        virtual void destroy_pipeline(PipelineHandle handle) = 0;

        virtual DescriptorSetLayoutHandle create_descriptor_set_layout(const DescriptorSetLayoutDescription& desc) = 0;
        virtual void destroy_descriptor_set_layout(DescriptorSetLayoutHandle handle) = 0;

        // Note: Descriptor sets are often allocated from a pool for efficiency.
        // This simple create function is a good starting point.
        virtual DescriptorSetHandle create_descriptor_set(const DescriptorSetDescription& desc) = 0;
        virtual void destroy_descriptor_set(DescriptorSetHandle handle) = 0; // Or free back to a pool

        virtual SamplerHandle create_sampler(const SamplerDescription& desc) = 0;
        virtual void destroy_sampler(SamplerHandle handle) = 0;

        virtual TextureHandle acquire_next_swapchain_image() = 0;

        virtual void present() = 0;

        virtual std::unique_ptr<CommandBuffer> create_command_buffer() = 0;

        virtual void submit(const std::vector<std::unique_ptr<CommandBuffer>> &command_buffers) = 0;

        virtual CommandBuffer *begin_frame() = 0;

        virtual void end_frame() = 0;

        virtual void wait_idle() = 0;
    };
} // namespace RAL
