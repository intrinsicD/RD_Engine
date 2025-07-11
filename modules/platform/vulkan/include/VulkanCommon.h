//vulkan/VulkanCommon.h
#pragma once

#include <cstdio>

namespace RDE {
#define RDE_USED_VK_VERSION VK_API_VERSION_1_3
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