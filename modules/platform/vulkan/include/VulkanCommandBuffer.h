//vulkan/VulkanCommandBuffer.h
#pragma once

#include "ral/CommandBuffer.h"

#include <vulkan/vulkan.h>

namespace RDE {
    class VulkanDevice; // Forward declare

    class VulkanCommandBuffer : public RAL::CommandBuffer {
    public:
        explicit VulkanCommandBuffer(VkCommandBuffer handle, VulkanDevice *device);

        ~VulkanCommandBuffer() override;

        // --- RAL Interface Implementation (all will be stubbed for now) ---
        void begin() override;

        void end() override;

        void begin_render_pass(const RAL::RenderPassDescription &desc) override;

        void end_render_pass() override;

        void set_viewport(const RAL::Viewport &viewport) override;

        void set_scissor(const RAL::Rect2D &scissor) override;

        void bind_pipeline(RAL::PipelineHandle pipeline) override;

        void bind_vertex_buffer(RAL::BufferHandle buffer, uint32_t binding) override;

        void bind_index_buffer(RAL::BufferHandle buffer, RAL::IndexType indexType) override;

        void bind_descriptor_set(RAL::PipelineHandle pipeline, RAL::DescriptorSetHandle set, uint32_t setIndex) override;

        void push_constants(RAL::PipelineHandle pipeline, RAL::ShaderStage stages, uint32_t offset, uint32_t size, const void* data) override;

        void draw(uint32_t vertex_count,
                  uint32_t instance_count,
                  uint32_t first_vertex,
                  uint32_t first_instance) override;

        void draw_indexed(uint32_t index_count,
                          uint32_t instance_count,
                          uint32_t first_index,
                          int32_t vertex_offset,
                          uint32_t first_instance) override;


        // --- Vulkan Specific ---
        VkCommandBuffer get_handle() const { return m_handle; }

    private:
        VkCommandBuffer m_handle = VK_NULL_HANDLE;
        VulkanDevice *m_device;
    };
}