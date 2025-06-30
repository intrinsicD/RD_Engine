#pragma once

#include "ral/CommandBuffer.h"

#include <vulkan/vulkan.h>

namespace RDE {
    class VulkanDevice; // Forward declare

    class VulkanCommandBuffer : public RAL::CommandBuffer {
    public:
        explicit VulkanCommandBuffer(VulkanDevice &device);

        ~VulkanCommandBuffer() override;

        // --- RAL Interface Implementation (all will be stubbed for now) ---
        void begin() override;

        void end() override;

        void begin_render_pass(const RAL::RenderPassBeginInfo &begin_info) override;

        void end_render_pass() override;

        void bind_pipeline(RAL::PipelineHandle pipeline) override;

        void bind_descriptor_set(uint32_t set_index, RAL::DescriptorSetHandle descriptor_set) override;

        void bind_vertex_buffer(uint32_t binding_index, RAL::BufferHandle buffer, uint64_t offset) override;

        void bind_index_buffer(RAL::BufferHandle buffer, RAL::IndexType index_type, uint64_t offset) override;

        void
        draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;

        void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                          uint32_t first_instance) override;

        void set_viewport(const RAL::Viewport &viewport) override;

        void set_scissor(const RAL::Rect2D &scissor) override;

        void pipeline_barrier(const RAL::BarrierInfo &barrier_info) override;

        void begin_debug_label(const std::string &label_name) override;

        void end_debug_label() override;

        void insert_debug_label(const std::string &label_name) override;
        // ... etc for all methods ...

        // --- Vulkan Specific ---
        VkCommandBuffer get_native_handle() const { return m_command_buffer; }

    private:
        VulkanDevice &m_device;
        VkCommandPool m_command_pool = VK_NULL_HANDLE;
        VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
    };
}