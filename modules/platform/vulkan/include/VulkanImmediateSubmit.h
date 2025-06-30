#pragma once

#include "VulkanDevice.h"

#include <functional>

namespace RDE {
    // A utility for performing immediate, one-off command submissions.
    // Useful for resource uploads and other setup tasks.
    class VulkanImmediateSubmit {
    public:
        VulkanImmediateSubmit(VulkanDevice& device);
        ~VulkanImmediateSubmit();

        void submit(std::function<void(RAL::CommandBuffer&)>&& function);

    private:
        VulkanDevice& m_device;
        VkFence m_fence = VK_NULL_HANDLE;
        std::unique_ptr<RAL::CommandBuffer> m_command_buffer;
    };
}