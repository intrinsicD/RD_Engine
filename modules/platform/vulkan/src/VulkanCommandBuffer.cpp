#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommon.h"
#include "VulkanResourceManager.h"

#include <cassert>


namespace RDE {
    VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer handle, VulkanDevice &device) : m_handle(handle),
                                                                                             m_device(device) {

    }

    VulkanCommandBuffer::~VulkanCommandBuffer() {

    }

    // --- RAL Interface Implementation (all will be stubbed for now) ---
    void VulkanCommandBuffer::begin() {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // This buffer will be submitted once and then reset.
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(m_handle, &beginInfo));
    }

    void VulkanCommandBuffer::end() {
        VK_CHECK(vkEndCommandBuffer(m_handle));
    }

    void VulkanCommandBuffer::begin_render_pass(const RAL::RenderPassDescription &desc) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderArea.offset = {0, 0};
        // By default, use the full swapchain extent. Could be overridden by desc.
        renderPassInfo.renderArea.extent = m_device.m_Swapchain.extent;

        // We only have one color attachment for now.
        const auto& colorAttachmentDesc = desc.colorAttachments[0];

        // --- NEW LOGIC ---
        if (!colorAttachmentDesc.texture.is_valid()) {
            // Handle is invalid, so we assume we're rendering to the swapchain.
            renderPassInfo.renderPass = m_device.m_SwapchainRenderPass;
            renderPassInfo.framebuffer = m_device.m_SwapchainFramebuffers[m_device.m_CurrentImageIndex];
        } else {
            // TODO: A more advanced path for rendering to an offscreen texture.
            // This would involve looking up the texture's VkFramebuffer from a resource manager.
            // For now, we can throw an error or assert.
            assert(false && "Rendering to specific textures not yet implemented.");
        }

        VkClearValue clearValue{};
        clearValue.color = {{ colorAttachmentDesc.clearColor[0], colorAttachmentDesc.clearColor[1], colorAttachmentDesc.clearColor[2], colorAttachmentDesc.clearColor[3] }};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(m_handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::end_render_pass() {
        vkCmdEndRenderPass(m_handle);
    }

    void VulkanCommandBuffer::set_viewport(const RAL::Viewport &viewport) {
        VkViewport vkViewport{
                .x = viewport.x,
                .y = viewport.y,
                .width = viewport.width,
                .height = viewport.height,
                .minDepth = viewport.min_depth,
                .maxDepth = viewport.max_depth
        };
        vkCmdSetViewport(m_handle, 0, 1, &vkViewport);
    }

    void VulkanCommandBuffer::set_scissor(const RAL::Rect2D &scissor) {
        VkRect2D vkScissor{
                .offset = {scissor.x, scissor.y},
                .extent = {scissor.width, scissor.height}
        };
        vkCmdSetScissor(m_handle, 0, 1, &vkScissor);
    }

    void VulkanCommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
                                   uint32_t first_instance) {
        vkCmdDraw(m_handle, vertex_count, instance_count, first_vertex, first_instance);
    }

    void VulkanCommandBuffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                                           int32_t vertex_offset, uint32_t first_instance) {
        vkCmdDrawIndexed(m_handle, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void VulkanCommandBuffer::bind_pipeline(RAL::PipelineHandle pipeline_handle) {
        VulkanPipeline& pipeline = m_device.m_PipelineManager.get(pipeline_handle);
        vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    }

    void VulkanCommandBuffer::bind_vertex_buffer(RAL::BufferHandle buffer_handle, uint32_t binding) {
        VulkanBuffer &buffer = m_device.m_BufferManager.get(buffer_handle);
        VkBuffer buffers[] = {buffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_handle, binding, 1, buffers, offsets);
    }

    void VulkanCommandBuffer::bind_index_buffer(RAL::BufferHandle buffer_handle, RAL::IndexType indexType) {
        VulkanBuffer &buffer = m_device.m_BufferManager.get(buffer_handle);
        VkIndexType vkIndexType = (indexType == RAL::IndexType::UINT16) ? VK_INDEX_TYPE_UINT16
                                                                        : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_handle, buffer.handle, 0, vkIndexType);
    }
}