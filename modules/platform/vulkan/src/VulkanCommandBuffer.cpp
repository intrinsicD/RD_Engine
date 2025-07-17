#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommon.h"
#include "VulkanResourceManager.h"
#include "VulkanMappers.h"

#include <cassert>

namespace RDE {
    VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer handle, VulkanDevice *device) : m_handle(handle),
                                                                                             m_device(device) {

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
        assert(!desc.colorAttachments.empty() && "Dynamic rendering requires at least one color attachment description.");

        const auto& colorAttachmentDesc = desc.colorAttachments[0];
        VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;

        auto &swapchain = m_device->get_swapchain();
        if (!colorAttachmentDesc.texture.is_valid()) {
            // Rendering to the swapchain
            colorAttachmentInfo.imageView = swapchain.get_current_image_view();
            assert(colorAttachmentInfo.imageView != VK_NULL_HANDLE && "Swapchain image view is null!");
        } else {
            // Rendering to an offscreen texture
            auto& texture = m_device->get_resources_database().get<VulkanTexture>(colorAttachmentDesc.texture);
            colorAttachmentInfo.imageView = texture.image_view;
        }



        colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp = (colorAttachmentDesc.loadOp == RAL::LoadOp::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo.storeOp = (colorAttachmentDesc.storeOp == RAL::StoreOp::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentInfo.clearValue.color = {{
                                                        colorAttachmentDesc.clearColor[0],
                                                        colorAttachmentDesc.clearColor[1],
                                                        colorAttachmentDesc.clearColor[2],
                                                        colorAttachmentDesc.clearColor[3]
                                                }};

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapchain.get_extent();
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachmentInfo;
        renderingInfo.pDepthAttachment = nullptr; // No depth yet
        renderingInfo.pStencilAttachment = nullptr;

        assert(renderingInfo.renderArea.extent.width > 0 && "Render area width cannot be zero!");
        assert(renderingInfo.renderArea.extent.height > 0 && "Render area height cannot be zero!");

        vkCmdBeginRendering(m_handle, &renderingInfo);
    }

    void VulkanCommandBuffer::end_render_pass() {
        vkCmdEndRendering(m_handle);
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
        auto &pipeline = m_device->get_resources_database().get<VulkanPipeline>(pipeline_handle);
        vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    }

    void VulkanCommandBuffer::bind_vertex_buffer(RAL::BufferHandle buffer_handle, uint32_t binding) {
        auto &buffer = m_device->get_resources_database().get<VulkanBuffer>(buffer_handle);
        VkBuffer buffers[] = {buffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_handle, binding, 1, buffers, offsets);
    }

    void VulkanCommandBuffer::bind_index_buffer(RAL::BufferHandle buffer_handle, RAL::IndexType indexType) {
        auto &buffer = m_device->get_resources_database().get<VulkanBuffer>(buffer_handle);
        VkIndexType vkIndexType = (indexType == RAL::IndexType::UINT16) ? VK_INDEX_TYPE_UINT16
                                                                        : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_handle, buffer.handle, 0, vkIndexType);
    }

    void VulkanCommandBuffer::bind_descriptor_set(RAL::PipelineHandle pipeline_handle, RAL::DescriptorSetHandle set_handle,
                                                  uint32_t setIndex) {
        auto &resources_db = m_device->get_resources_database();
        // Assert that the handles are valid before proceeding.
        // In a release build, these checks might be compiled out.
        assert(resources_db.is_valid(pipeline_handle) && "Invalid pipeline handle provided to bind_descriptor_set");
        assert(resources_db.is_valid(set_handle) && "Invalid descriptor set handle provided to bind_descriptor_set");

        // 1. Get the concrete Vulkan pipeline layout from the pipeline handle.
        // The layout defines the "shape" that the descriptor sets must conform to.
        auto& pipeline = resources_db.get<VulkanPipeline>(pipeline_handle);
        VkPipelineLayout pipelineLayout = pipeline.layout;

        // 2. Get the concrete VkDescriptorSet from our RAL handle.
        auto &vkSet = resources_db.get<VulkanDescriptorSet>(set_handle);

        // 3. Record the command.
        vkCmdBindDescriptorSets(
                m_handle,                         // The VkCommandBuffer
                VK_PIPELINE_BIND_POINT_GRAPHICS,  // We're binding for a graphics pipeline
                pipelineLayout,                   // The layout the pipeline was created with
                setIndex,                         // The set index to bind to (e.g., set=0, set=1 in GLSL)
                1,                                // We are binding one set at a time
                &vkSet.handle,                           // Pointer to the descriptor set handle
                0,                                // No dynamic offsets (an advanced feature)
                nullptr                           // No dynamic offsets
        );
    }

    void VulkanCommandBuffer::push_constants(RAL::PipelineHandle pipeline_handle, RAL::ShaderStage stages, uint32_t offset,
                                             uint32_t size, const void *data) {
        assert(m_device->get_resources_database().is_valid(pipeline_handle) && "Invalid pipeline handle provided to push_constants");
        assert(data != nullptr && "Data pointer for push_constants cannot be null");
        assert(size > 0 && "Push constant size must be greater than 0");

        // 1. Get the pipeline layout, as this is what the push constants are associated with.
        auto& pipeline = m_device->get_resources_database().get<VulkanPipeline>(pipeline_handle);
        VkPipelineLayout pipelineLayout = pipeline.layout;

        // 2. Convert our RAL shader stage bitmask to Vulkan's bitmask.
        // This helper function should already exist from our previous work.
        VkShaderStageFlags vkStageFlags = ToVulkanShaderStageFlags(stages);

        // 3. Record the command.
        vkCmdPushConstants(
                m_handle,        // The VkCommandBuffer
                pipelineLayout,  // The layout defining the push constant ranges
                vkStageFlags,    // The shader stage(s) that will access the data
                offset,          // The byte offset within the push constant block
                size,            // The size of the data to update
                data             // Pointer to the data
        );
    }

    void VulkanCommandBuffer::transition_image_layout(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = current_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        // Define access masks and pipeline stages for the transition
        if (current_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (current_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else {
            // You can add more transition types as needed
            throw std::runtime_error("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(m_handle, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }
}