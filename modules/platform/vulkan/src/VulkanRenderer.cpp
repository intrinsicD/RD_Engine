#include "VulkanRenderer.h"
#include "VulkanCommandBuffer.h"
#include "AssetDatabase.h"

namespace RDE {

    VulkanRenderer::VulkanRenderer() {
        m_device = std::make_unique<VulkanDevice>();
    }

    VulkanRenderer::~VulkanRenderer() {
        // unique_ptr will handle destruction in the correct order.
        // We might need to call device->wait_idle() here to be safe.
        if (m_device) {
            m_device->wait_idle();
        }
        m_device.reset();
    }

    FrameContext VulkanRenderer::begin_frame() {
        // RDE_CORE_ASSERT(!m_frame_started, "Cannot call begin_frame while another frame is in progress!");

        // Get the resources for the frame we are about to render.
        FrameData& current_frame_data = m_device->get_current_frame_data();

        // 1. Wait for the GPU to finish the frame that was previously using these resources.
        vkWaitForFences(m_device->get_logical_device(), 1, &current_frame_data.in_flight_fence, VK_TRUE, UINT64_MAX);

        // 2. Acquire an image from the swap chain.
        VkResult result = m_device->acquire_next_swapchain_image(&m_current_swapchain_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_device->recreate_swapchain();
            return { .is_valid = false }; // App should skip this frame.
        }

        // 3. Now that we know we're using this frame's resources, reset the fence and command pool.
        vkResetFences(m_device->get_logical_device(), 1, &current_frame_data.in_flight_fence);
        vkResetCommandPool(m_device->get_logical_device(), current_frame_data.command_pool, 0);

        // 4. Begin the primary command buffer for this frame.
        VkCommandBuffer cmd_buffer_handle = current_frame_data.primary_command_buffer;
        VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd_buffer_handle, &begin_info);

        // 5. Return the context to the application.
        m_frame_started = true;
        return {
                .is_valid = true,
                .command_buffer = current_frame_data.ral_command_buffer.get(),
                // .backbuffer = m_device->get_swapchain_texture(m_current_swapchain_image_index), // You'd implement this getter
                .frame_index = m_device->get_current_frame_index()
        };
    }

    void VulkanRenderer::submit_and_present(FrameContext &context) {
        // RDE_CORE_ASSERT(m_frame_started, "Cannot call submit_and_present before begin_frame!");
        m_frame_started = false;

        // 1. End the command buffer that the application was recording into.
        vkEndCommandBuffer(context.command_buffer->get_native_handle());

        // 2. Submit the command buffer to the GPU.
        m_device->submit();

        // 3. Present the resulting image to the swapchain.
        m_device->on_present(m_current_swapchain_image_index);

        // Handle potential resize on present
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            m_device->recreate_swapchain();
        }

        // 4. Move to the next frame in flight.
        m_device->advance_frame();
    }
} // namespace RDE