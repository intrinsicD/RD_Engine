#include "VulkanImmediateSubmit.h"

#include <stdexcept>

namespace RDE {
    VulkanImmediateSubmit::VulkanImmediateSubmit(VulkanDevice& device) : m_device(device) {
        // We need a VulkanCommandBuffer, so we must downcast.
        // This is one of the few places where a cast is acceptable, as it's
        // part of the concrete backend implementation.
        m_command_buffer = m_device.create_command_buffer();

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled

        if (vkCreateFence(m_device.get_logical_device(), &fence_create_info, nullptr, &m_fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence for immediate submit!");
        }
    }

    VulkanImmediateSubmit::~VulkanImmediateSubmit() {
        vkDestroyFence(m_device.get_logical_device(), m_fence, nullptr);
    }

    void VulkanImmediateSubmit::submit(std::function<void(RAL::CommandBuffer&)>&& function) {
        VkDevice logical_device = m_device.get_logical_device();

        // Wait for the fence to ensure the previous submission is complete
        vkWaitForFences(logical_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(logical_device, 1, &m_fence);

        m_command_buffer->begin();
        function(*m_command_buffer);
        m_command_buffer->end();

        m_device.submit_command_buffers({ m_command_buffer.get() }); // <-- Assume this submits to graphics queue and signals a fence

        // In a real implementation, submit_command_buffers would take an optional fence.
        // We will assume for now it uses the one we give it, or we wait on a queue.
        // For simplicity here, we'll just wait for the device to be idle.
        // THIS IS INEFFICIENT but fine for one-off uploads.
        m_device.wait_idle();
    }
}