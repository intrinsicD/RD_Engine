//ral/Device.h
#pragma once

#include "Common.h"
#include "CommandBuffer.h"

#include <memory>
#include <vector>
#include <functional>

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

        virtual CommandBuffer *begin_frame() = 0;

        virtual void end_frame(const std::vector<RAL::CommandBuffer *> &command_buffers) = 0;

        virtual void wait_idle() = 0;

        virtual void recreate_swapchain() = 0;

        virtual BufferHandle create_buffer(const BufferDescription &desc) = 0;


        virtual void destroy_buffer(BufferHandle handle) = 0;

        virtual TextureHandle create_texture(const TextureDescription &desc) = 0;


        virtual void destroy_texture(TextureHandle handle) = 0;

        virtual ShaderHandle create_shader(const ShaderDescription &desc) = 0;

        virtual ShaderHandle create_shader_module(const std::vector<char> &bytecode, ShaderStage stage) = 0;

        virtual void destroy_shader(ShaderHandle handle) = 0;

        virtual PipelineHandle create_pipeline(const PipelineDescription &desc) = 0;

        virtual void destroy_pipeline(PipelineHandle handle) = 0;

        virtual DescriptorSetLayoutHandle create_descriptor_set_layout(const DescriptorSetLayoutDescription &desc) = 0;

        virtual void destroy_descriptor_set_layout(DescriptorSetLayoutHandle handle) = 0;

        virtual DescriptorSetHandle create_descriptor_set(const DescriptorSetDescription &desc) = 0;

        virtual void destroy_descriptor_set(DescriptorSetHandle handle) = 0; // Or free back to a pool

        virtual SamplerHandle create_sampler(const SamplerDescription &desc) = 0;
        
        virtual void destroy_sampler(SamplerHandle handle) = 0;

        virtual void *map_buffer(BufferHandle handle) = 0;

        virtual void unmap_buffer(BufferHandle handle) = 0;

        virtual void update_buffer_data(RAL::BufferHandle targetBuffer, const void *data, size_t size, size_t offset) = 0;

        virtual void copy_buffer(RAL::BufferHandle source, RAL::BufferHandle target, size_t size, size_t source_offset, size_t target_offset) = 0;

    };
} // namespace RAL
