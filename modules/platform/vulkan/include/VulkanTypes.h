//vulkan/VulkanTypes.h
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace RDE {
    struct VulkanBuffer {
        VkBuffer handle = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        size_t size = 0;
        RAL::MemoryUsage memoryUsage = RAL::MemoryUsage::DeviceLocal; // Default to DeviceLocal, can be changed later
    };

    struct VulkanTexture {
        VkImage handle = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
    };

    struct VulkanShader {
        VkShaderModule module{VK_NULL_HANDLE};
    };

    struct VulkanPipeline {
        VkPipeline handle{VK_NULL_HANDLE};
        VkPipelineLayout layout{VK_NULL_HANDLE};
    };

    struct VulkanSampler {
        VkSampler handle{VK_NULL_HANDLE};
    };

    struct VulkanDescriptorSetLayout {
        VkDescriptorSetLayout handle{VK_NULL_HANDLE};
    };

    struct VulkanDescriptorSet {
        VkDescriptorSet handle{VK_NULL_HANDLE};
    };
    // ... other concrete types later
}