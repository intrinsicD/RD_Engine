//ral/Device.h
#pragma once

#include "Common.h"
#include "CommandBuffer.h"
#include "ResourcesDatabase.h"

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

    struct FrameContext {
        // The handle to the swapchain texture you should render to for this frame.
        TextureHandle swapchainTexture;
        uint32_t frameIndex; // Index of the current frame in flight
        // Internal sync primitives would be here (e.g., semaphores) but are hidden from the user.
        uint32_t swapchainImageIndex;
    };

    class Device {
    public:
        virtual ~Device() = default;

        virtual FrameContext begin_frame() = 0;

        virtual void end_frame(const FrameContext &context, const std::vector<CommandBuffer *> &command_buffers) = 0;

        virtual CommandBuffer* get_command_buffer() = 0;

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

        virtual void update_buffer_data(BufferHandle targetBuffer, const void *data, size_t size, size_t offset) = 0;

        virtual void immediate_submit(std::function<void(CommandBuffer* cmd)>&& function) = 0;

        virtual void submit_and_wait(const std::vector<RAL::CommandBuffer*>& command_buffers) = 0;

        ResourcesDatabase &get_resources_database() {
            return m_resources_db;
        }

        const ResourcesDatabase &get_resources_database() const {
            return m_resources_db;
        }
    protected:
        ResourcesDatabase m_resources_db;
    };
} // namespace RAL
