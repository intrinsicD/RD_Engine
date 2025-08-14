#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommon.h"
#include "VulkanTypes.h"
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
        // Auto-close an open dynamic rendering pass to guarantee present transition path
        if(m_inRenderPass) {
            end_render_pass();
        }
        VK_CHECK(vkEndCommandBuffer(m_handle));
    }

    void VulkanCommandBuffer::begin_render_pass(const RAL::RenderPassDescription &desc) {
        m_inRenderPass = true; // NEW
        // --- Step 1: Validate that there is something to render to. ---
        assert(!desc.colorAttachments.empty() || desc.depthStencilAttachment.texture.is_valid() &&
                                                 "Render pass must have at least one color or depth attachment.");

        auto &db = m_device->get_resources_database();
        m_currentColorAttachments.clear(); // NEW
        // Layout transition barriers (image) collected first
        std::vector<VkImageMemoryBarrier> imageBarriers; // NEW
        std::vector<RAL::TextureHandle> imagesToUpdateLayout; // NEW: defer layout state update until after barrier
        auto add_transition = [&](RAL::TextureHandle texHandle, RAL::ImageLayout desiredLayout, bool isDepth){
            if(!texHandle.is_valid()) return;
            auto &vkTex = db.get<VulkanTexture>(texHandle);
            if(vkTex.currentLayout == desiredLayout) return; // already correct
            const auto &ralDesc = db.get<RAL::TextureDescription>(texHandle);
            VkImageMemoryBarrier b{}; b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            b.oldLayout = ToVulkanImageLayout(vkTex.currentLayout);
            b.newLayout = ToVulkanImageLayout(desiredLayout);
            b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            b.image = vkTex.handle;
            // Removed incorrect override of oldLayout for swapchain images; must use tracked state (Undefined first use)
            if (has_flag(ralDesc.usage, RAL::TextureUsage::DepthStencilAttachment)) {
                b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                if (ralDesc.format == RAL::Format::D24_UNORM_S8_UINT || ralDesc.format == RAL::Format::D32_SFLOAT_S8_UINT)
                    b.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            } else {
                b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            b.subresourceRange.baseMipLevel = 0; b.subresourceRange.levelCount = ralDesc.mipLevels;
            b.subresourceRange.baseArrayLayer = 0; b.subresourceRange.layerCount = 1;
            // Conservative src/dst access + stages (we will use TOP_OF_PIPE -> appropriate below)
            if (isDepth) {
                b.srcAccessMask = 0;
                b.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            } else {
                b.srcAccessMask = 0;
                b.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            imageBarriers.push_back(b);
            imagesToUpdateLayout.push_back(texHandle); // defer state update
        };
        for (const auto &c: desc.colorAttachments) {
            add_transition(c.texture, RAL::ImageLayout::ColorAttachment, false);
        }
        if (desc.depthStencilAttachment.texture.is_valid()) {
            add_transition(desc.depthStencilAttachment.texture, RAL::ImageLayout::DepthStencilAttachment, true);
        }
        if (!imageBarriers.empty()) {
            vkCmdPipelineBarrier(
                    m_handle,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data()
            );
            // Now that the barrier is recorded, update tracked layouts to the new state.
            for (auto h: imagesToUpdateLayout) {
                auto &vkTex = db.get<VulkanTexture>(h);
                // Determine target layout we requested above
                // (We can map from current barrier list but simpler: use desired inferred from render pass role)
                // If it was depth attachment, set DepthStencilAttachment else ColorAttachment.
                // Exact desired tracked in add_transition invocation order; reuse logic quickly:
                // We know new layout encoded in barrier already executed; derive from first matching barrier.
                // Simplicity: find barrier with same image handle.
                for (const auto &b: imageBarriers) {
                    if (b.image == vkTex.handle) {
                        // Map VkImageLayout back to RAL::ImageLayout
                        if (b.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                            vkTex.currentLayout = RAL::ImageLayout::ColorAttachment;
                        else if (b.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                            vkTex.currentLayout = RAL::ImageLayout::DepthStencilAttachment;
                        else if (b.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                            vkTex.currentLayout = RAL::ImageLayout::PresentSrc;
                        break;
                    }
                }
            }
        }

        // --- Step 2: Prepare storage for Vulkan attachment descriptions. ---
        // These vectors must stay in scope until vkCmdBeginRendering is called.
        std::vector<VkRenderingAttachmentInfoKHR> vkColorAttachments;
        vkColorAttachments.reserve(desc.colorAttachments.size());

        VkRenderingAttachmentInfoKHR vkDepthAttachment{}; // Zero-initialized

        // --- Step 3: Process all Color Attachments from the RAL description. ---
        for (const auto &ralColorAttachment: desc.colorAttachments) {
            // record handle
            m_currentColorAttachments.push_back(ralColorAttachment.texture); // NEW
            assert(ralColorAttachment.texture.is_valid() && "Color attachment texture handle is invalid.");
            const auto &vkTexture = db.get<VulkanTexture>(ralColorAttachment.texture);

            VkRenderingAttachmentInfoKHR attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            attachmentInfo.imageView = vkTexture.image_view;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = ToVulkanLoadOp(ralColorAttachment.loadOp); // You'll need a ToVulkanLoadOp mapper
            attachmentInfo.storeOp = ToVulkanStoreOp(ralColorAttachment.storeOp); // And a ToVulkanStoreOp mapper
            attachmentInfo.clearValue.color = {
                    {
                            ralColorAttachment.clearColor[0],
                            ralColorAttachment.clearColor[1],
                            ralColorAttachment.clearColor[2],
                            ralColorAttachment.clearColor[3]
                    }
            };

            vkColorAttachments.push_back(attachmentInfo);
        }

        // --- Step 4: Process the Depth/Stencil Attachment, if it exists. ---
        if (desc.depthStencilAttachment.texture.is_valid()) {
            const auto &vkTexture = db.get<VulkanTexture>(desc.depthStencilAttachment.texture);

            vkDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            vkDepthAttachment.imageView = vkTexture.image_view;
            vkDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            vkDepthAttachment.loadOp = ToVulkanLoadOp(desc.depthStencilAttachment.loadOp);
            vkDepthAttachment.storeOp = ToVulkanStoreOp(desc.depthStencilAttachment.storeOp);
            vkDepthAttachment.clearValue.depthStencil = {
                    desc.depthStencilAttachment.clearDepth,
                    desc.depthStencilAttachment.clearStencil
            };
        }

        // --- Step 5: Determine the render area from the attachments themselves. ---
        // This is CRITICAL. The render area should match the target, not always the swapchain.
        VkExtent2D renderArea = {};
        if (!desc.colorAttachments.empty()) {
            const auto &firstAttachmentDesc = db.get<RAL::TextureDescription>(desc.colorAttachments[0].texture);
            renderArea = {firstAttachmentDesc.width, firstAttachmentDesc.height};
        } else {
            // If no color, must have depth.
            const auto &depthAttachmentDesc = db.get<RAL::TextureDescription>(desc.depthStencilAttachment.texture);
            renderArea = {depthAttachmentDesc.width, depthAttachmentDesc.height};
        }

        assert(renderArea.width > 0 && "Render area width cannot be zero!");
        assert(renderArea.height > 0 && "Render area height cannot be zero!");

        // --- Step 6: Assemble the final VkRenderingInfo struct. ---
        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea = {{0, 0}, renderArea};
        renderingInfo.layerCount = 1;
        renderingInfo.viewMask = 0; // Not using multiview

        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(vkColorAttachments.size());
        renderingInfo.pColorAttachments = vkColorAttachments.empty() ? nullptr : vkColorAttachments.data();

        if (desc.depthStencilAttachment.texture.is_valid()) {
            renderingInfo.pDepthAttachment = &vkDepthAttachment;

            // If the format has a stencil component, point pStencilAttachment to the same struct.
            const auto &depthDesc = db.get<RAL::TextureDescription>(desc.depthStencilAttachment.texture);
            if (depthDesc.format == RAL::Format::D24_UNORM_S8_UINT || depthDesc.format ==
                                                                      RAL::Format::D32_SFLOAT_S8_UINT) {
                renderingInfo.pStencilAttachment = &vkDepthAttachment;
            } else {
                renderingInfo.pStencilAttachment = nullptr;
            }
        } else {
            renderingInfo.pDepthAttachment = nullptr;
            renderingInfo.pStencilAttachment = nullptr;
        }

        // --- Step 7: Record the command. ---
        vkCmdBeginRendering(m_handle, &renderingInfo);
    }

    void VulkanCommandBuffer::end_render_pass() {
        if(!m_inRenderPass) return; // guard
        m_inRenderPass = false;
        vkCmdEndRendering(m_handle);
        // Transition swapchain color attachments back to PRESENT_SRC_KHR (isSwapchainImage flag)
        if (!m_currentColorAttachments.empty()) {
            auto &db = m_device->get_resources_database();
            std::vector<VkImageMemoryBarrier> presentBarriers;
            for (auto h: m_currentColorAttachments) {
                if (!h.is_valid()) continue;
                auto &vkTex = db.get<VulkanTexture>(h);
                if (vkTex.isSwapchainImage && vkTex.currentLayout == RAL::ImageLayout::ColorAttachment) {
                    VkImageMemoryBarrier b{}; b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    b.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    b.dstAccessMask = 0;
                    b.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    b.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    b.image = vkTex.handle;
                    b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    b.subresourceRange.baseMipLevel = 0; b.subresourceRange.levelCount = 1;
                    b.subresourceRange.baseArrayLayer = 0; b.subresourceRange.layerCount = 1;
                    presentBarriers.push_back(b);
                    vkTex.currentLayout = RAL::ImageLayout::PresentSrc; // update tracking
                }
            }
            if (!presentBarriers.empty()) {
                vkCmdPipelineBarrier(
                        m_handle,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        static_cast<uint32_t>(presentBarriers.size()), presentBarriers.data()
                );
            }
        }
        m_currentColorAttachments.clear();
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
        vkCmdBindPipeline(m_handle, pipeline.bindPoint, pipeline.handle); // UPDATED
    }

    void VulkanCommandBuffer::pipeline_barrier(const RAL::ResourceBarrier &barrier) {
        VkImageMemoryBarrier imageBarrier{};
        bool hasImage = false;
        if (barrier.textureTransition.texture.is_valid()) {
            auto &db = m_device->get_resources_database();
            auto &vkTexture = db.get<VulkanTexture>(barrier.textureTransition.texture);
            const auto &ralDesc = db.get<RAL::TextureDescription>(barrier.textureTransition.texture);

            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.srcAccessMask = ToVulkanAccessFlags(barrier.srcAccess);
            imageBarrier.dstAccessMask = ToVulkanAccessFlags(barrier.dstAccess);
            // Override oldLayout with tracked layout to avoid stale/mismatched user input.
            auto trackedOld = vkTexture.currentLayout;
            if (barrier.textureTransition.oldLayout != trackedOld && barrier.textureTransition.oldLayout != RAL::ImageLayout::Undefined) {
                // Optional: log mismatch (could be noisy)
                // RDE_CORE_WARN("pipeline_barrier: overriding user oldLayout ({})-> tracked ({})", (int)barrier.textureTransition.oldLayout, (int)trackedOld);
            }
            imageBarrier.oldLayout = ToVulkanImageLayout(trackedOld);
            imageBarrier.newLayout = ToVulkanImageLayout(barrier.textureTransition.newLayout);
            imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.image = vkTexture.handle;

            if (has_flag(ralDesc.usage, RAL::TextureUsage::DepthStencilAttachment)) {
                imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                if (ralDesc.format == RAL::Format::D24_UNORM_S8_UINT || ralDesc.format == RAL::Format::D32_SFLOAT_S8_UINT)
                    imageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            } else {
                imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            imageBarrier.subresourceRange.baseMipLevel = 0;
            imageBarrier.subresourceRange.levelCount = ralDesc.mipLevels;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount = 1;
            hasImage = true;
        }

        VkMemoryBarrier memoryBarrier{};
        bool hasMemory = false;
        if (!barrier.textureTransition.texture.is_valid()) {
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier.srcAccessMask = ToVulkanAccessFlags(barrier.srcAccess);
            memoryBarrier.dstAccessMask = ToVulkanAccessFlags(barrier.dstAccess);
            hasMemory = true;
        }

        if (hasImage || hasMemory) {
            vkCmdPipelineBarrier(
                m_handle,
                ToVulkanPipelineStageFlags(barrier.srcStage),
                ToVulkanPipelineStageFlags(barrier.dstStage),
                0,
                hasMemory ? 1u : 0u, hasMemory ? &memoryBarrier : nullptr,
                0, nullptr,
                hasImage ? 1u : 0u, hasImage ? &imageBarrier : nullptr
            );
        }

        if (barrier.textureTransition.texture.is_valid()) {
            auto &vkTexture = m_device->get_resources_database().get<VulkanTexture>(barrier.textureTransition.texture);
            vkTexture.currentLayout = barrier.textureTransition.newLayout;
        }
    }

    void VulkanCommandBuffer::bind_vertex_buffer(RAL::BufferHandle buffer_handle, uint32_t binding) {
        auto &buffer = m_device->get_resources_database().get<VulkanBuffer>(buffer_handle);
        VkBuffer buffers[] = {buffer.handle};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_handle, binding, 1, buffers, offsets);
    }

    void VulkanCommandBuffer::bind_index_buffer(RAL::BufferHandle buffer_handle, RAL::IndexType indexType) {
        auto &buffer = m_device->get_resources_database().get<VulkanBuffer>(buffer_handle);
        VkIndexType vkIndexType = (indexType == RAL::IndexType::UINT16)
                                  ? VK_INDEX_TYPE_UINT16
                                  : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_handle, buffer.handle, 0, vkIndexType);
    }

    void VulkanCommandBuffer::bind_descriptor_set(RAL::PipelineHandle pipeline_handle,
                                                  RAL::DescriptorSetHandle set_handle,
                                                  uint32_t setIndex) {
        auto &resources_db = m_device->get_resources_database();
        // Assert that the handles are valid before proceeding.
        // In a release build, these checks might be compiled out.
        assert(resources_db.is_valid(pipeline_handle) && "Invalid pipeline handle provided to bind_descriptor_set");
        assert(resources_db.is_valid(set_handle) && "Invalid descriptor set handle provided to bind_descriptor_set");

        // 1. Get the concrete Vulkan pipeline layout from the pipeline handle.
        // The layout defines the "shape" that the descriptor sets must conform to.
        auto &pipeline = resources_db.get<VulkanPipeline>(pipeline_handle);
        VkPipelineLayout pipelineLayout = pipeline.layout;

        // 2. Get the concrete VkDescriptorSet from our RAL handle.
        auto &vkSet = resources_db.get<VulkanDescriptorSet>(set_handle);

        // 3. Record the command.
        vkCmdBindDescriptorSets(
                m_handle, // The VkCommandBuffer
                VK_PIPELINE_BIND_POINT_GRAPHICS, // We're binding for a graphics pipeline
                pipelineLayout, // The layout the pipeline was created with
                setIndex, // The set index to bind to (e.g., set=0, set=1 in GLSL)
                1, // We are binding one set at a time
                &vkSet.handle, // Pointer to the descriptor set handle
                0, // No dynamic offsets (an advanced feature)
                nullptr // No dynamic offsets
        );
    }

    void VulkanCommandBuffer::copy_buffer(RAL::BufferHandle srcHandle, RAL::BufferHandle dstHandle, uint64_t size,
                                          uint64_t srcOffset, uint64_t dstOffset) {
        auto &db = m_device->get_resources_database();
        const auto &srcBuffer = db.get<VulkanBuffer>(srcHandle);
        const auto &dstBuffer = db.get<VulkanBuffer>(dstHandle);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;
        vkCmdCopyBuffer(m_handle, srcBuffer.handle, dstBuffer.handle, 1, &copyRegion);
    }

    void VulkanCommandBuffer::copy_buffer_to_texture(RAL::BufferHandle srcHandle, RAL::TextureHandle dstHandle,
                                                     const std::vector<RAL::BufferTextureCopy> &regions) {
        auto &db = m_device->get_resources_database();
        const auto &srcBuffer = db.get<VulkanBuffer>(srcHandle);
        const auto &dstTexture = db.get<VulkanTexture>(dstHandle);

        std::vector<VkBufferImageCopy> vkRegions;
        vkRegions.reserve(regions.size());

        for (const auto &region: regions) {
            VkBufferImageCopy vkRegion{};
            vkRegion.bufferOffset = region.bufferOffset;
            vkRegion.bufferRowLength = region.bufferRowLength;
            vkRegion.bufferImageHeight = region.bufferImageHeight;

            // Translate your RAL::ImageSubresourceLayers to VkImageSubresourceLayers
            vkRegion.imageSubresource.aspectMask = translate_aspect_mask(region.imageSubresource.aspectMask);
            vkRegion.imageSubresource.mipLevel = region.imageSubresource.mipLevel;
            vkRegion.imageSubresource.baseArrayLayer = region.imageSubresource.baseArrayLayer;
            vkRegion.imageSubresource.layerCount = region.imageSubresource.layerCount;

            vkRegion.imageOffset = {region.imageOffset.x, region.imageOffset.y, region.imageOffset.z};
            vkRegion.imageExtent = {region.imageExtent.width, region.imageExtent.height, region.imageExtent.depth};

            vkRegions.push_back(vkRegion);
        }

        if (!vkRegions.empty()) {
            vkCmdCopyBufferToImage(
                    m_handle,
                    srcBuffer.handle,
                    dstTexture.handle,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    static_cast<uint32_t>(vkRegions.size()),
                    vkRegions.data()
            );
        }
    }

    void VulkanCommandBuffer::push_constants(RAL::PipelineHandle pipeline_handle, RAL::ShaderStage stages,
                                             uint32_t offset,
                                             uint32_t size, const void *data) {
        assert(
                m_device->get_resources_database().is_valid(pipeline_handle) &&
                "Invalid pipeline handle provided to push_constants");
        assert(data != nullptr && "Data pointer for push_constants cannot be null");
        assert(size > 0 && "Push constant size must be greater than 0");

        // 1. Get the pipeline layout, as this is what the push constants are associated with.
        auto &pipeline = m_device->get_resources_database().get<VulkanPipeline>(pipeline_handle);
        VkPipelineLayout pipelineLayout = pipeline.layout;

        // 2. Convert our RAL shader stage bitmask to Vulkan's bitmask.
        // This helper function should already exist from our previous work.
        VkShaderStageFlags vkStageFlags = ToVulkanShaderStageFlags(stages);

        // 3. Record the command.
        vkCmdPushConstants(
                m_handle, // The VkCommandBuffer
                pipelineLayout, // The layout defining the push constant ranges
                vkStageFlags, // The shader stage(s) that will access the data
                offset, // The byte offset within the push constant block
                size, // The size of the data to update
                data // Pointer to the data
        );
    }

    void VulkanCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        vkCmdDispatch(m_handle, groupCountX, groupCountY, groupCountZ);
    }
}
