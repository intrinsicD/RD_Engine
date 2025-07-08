//vulkan/VulkanCommon.h
#pragma once

namespace RDE {
    // A simple macro for Vulkan call error handling
#define VK_CHECK(x)                                                 \
        do                                                              \
        {                                                               \
            VkResult err = x;                                           \
            if (err)                                                    \
            {                                                           \
                /* In a real engine, use a proper logging system */     \
                printf("Detected Vulkan error: %d\n", err);             \
                abort();                                                \
            }                                                           \
        } while (0)
}