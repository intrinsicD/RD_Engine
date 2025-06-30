#pragma once

#include "ral/Resources.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace RDE::VulkanMappers {

    // Buffers

    inline VkBufferUsageFlags ToVkBufferUsage(const RAL::BufferDescription& desc) {
        // For now, a simple mapping. A real engine would have more granular flags
        // in the RAL description (e.g., IsVertexBuffer, IsUniformBuffer).
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT | // All buffers can be copied to
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    inline VmaMemoryUsage ToVmaMemoryUsage(const RAL::BufferDescription& desc) {
        switch (desc.usage) {
            case RAL::ResourceUsage::STATIC:
                // Device local, highest performance for GPU access.
                return VMA_MEMORY_USAGE_GPU_ONLY;
            case RAL::ResourceUsage::DYNAMIC:
                // Host visible and coherent, best for frequent CPU updates.
                return VMA_MEMORY_USAGE_CPU_TO_GPU;
            case RAL::ResourceUsage::GPU_ONLY:
                return VMA_MEMORY_USAGE_GPU_ONLY;
            default:
                return VMA_MEMORY_USAGE_UNKNOWN;
        }
    }

    // Textures

    inline VkFormat ToVkFormat(RAL::Format format) {
        // This would be a large switch statement.
        switch (format) {
            case RAL::Format::R8G8B8A8_UNORM:   return VK_FORMAT_R8G8B8A8_UNORM;
            case RAL::Format::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case RAL::Format::D32_SFLOAT:        return VK_FORMAT_D32_SFLOAT;
                // ... map ALL formats from your RAL enum ...
            default:
                // RDE_CORE_ASSERT(false, "Unsupported texture format!");
                return VK_FORMAT_UNDEFINED;
        }
    }

    inline VkImageUsageFlags ToVkImageUsage(const RAL::TextureDescription& desc) {
        // A basic mapping. A real engine might have more granular usage flags in the RAL description.
        VkImageUsageFlags usage = 0;
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;          // All textures can be sampled in a shader
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;     // All textures can be a destination for a copy
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;     // All textures can be a source for a copy

        // Check if it's a depth/stencil format
        bool is_depth = (desc.format == RAL::Format::D32_SFLOAT || desc.format == RAL::Format::D24_UNORM_S8_UINT);

        if (is_depth) {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Can be used as a render target
        }

        // Add VK_IMAGE_USAGE_STORAGE_BIT if it can be used as a storage image

        return usage;
    }

    // VMA memory usage mapper is the same as for buffers, so we can reuse it.
    inline VmaMemoryUsage ToVmaMemoryUsage(const RAL::TextureDescription& desc) {
        switch (desc.usage) {
            case RAL::ResourceUsage::STATIC:   return VMA_MEMORY_USAGE_GPU_ONLY;
            case RAL::ResourceUsage::DYNAMIC:  return VMA_MEMORY_USAGE_CPU_TO_GPU; // Less common for textures
            case RAL::ResourceUsage::GPU_ONLY: return VMA_MEMORY_USAGE_GPU_ONLY;
            default:                           return VMA_MEMORY_USAGE_UNKNOWN;
        }
    }

} // namespace RDE::VulkanMappers