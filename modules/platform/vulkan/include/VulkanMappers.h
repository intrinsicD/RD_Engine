#pragma once

#include "ral/Common.h"
#include "ral/Resources.h"

#include <vulkan/vulkan.h>
#include <stdexcept>

#include <vk_mem_alloc.h>

namespace RDE {
    inline VkFormat ToVulkanFormat(RAL::Format format) {
        // This would be a large switch statement.
        // Let's add a few examples.
        switch (format) {
            // 8-bit
            case RAL::Format::R8_UNORM: return VK_FORMAT_R8_UNORM;
            case RAL::Format::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
            case RAL::Format::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
            case RAL::Format::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
            case RAL::Format::R8_SRGB: return VK_FORMAT_R8_SRGB;
            case RAL::Format::R8G8_SRGB: return VK_FORMAT_R8G8_SRGB;
            case RAL::Format::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
            case RAL::Format::B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
            // 16-bit
            case RAL::Format::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
            case RAL::Format::R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
            case RAL::Format::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
            // 32-bit
            case RAL::Format::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
            case RAL::Format::R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
            case RAL::Format::R32G32B32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
            case RAL::Format::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case RAL::Format::R32_UINT: return VK_FORMAT_R32_UINT;
            case RAL::Format::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
            case RAL::Format::R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
            case RAL::Format::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
            // Depth
            case RAL::Format::D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
            case RAL::Format::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
            case RAL::Format::D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            // Block Compression
            case RAL::Format::BC1_RGB_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case RAL::Format::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
            case RAL::Format::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;

            case RAL::Format::UNKNOWN:
            default:
                // You should have a logging system here
                // For now, we'll assert or throw
                throw std::runtime_error("Unsupported or unknown RAL::Format!");
                return VK_FORMAT_UNDEFINED;
        }
    }

    inline VkFilter ToVulkanFilter(RAL::Filter filter) {
        return filter == RAL::Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }

    inline VkSamplerAddressMode ToVulkanAddressMode(RAL::SamplerAddressMode mode) {
        switch (mode) {
            case RAL::SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case RAL::SamplerAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case RAL::SamplerAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case RAL::SamplerAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    inline VkDescriptorType ToVulkanDescriptorType(RAL::DescriptorType type) {
        switch (type) {
            case RAL::DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case RAL::DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    inline VkShaderStageFlags ToVulkanShaderStageFlags(RAL::ShaderStage stages) {
        VkShaderStageFlags flags = 0;
        if (has_flag(stages, RAL::ShaderStage::Vertex)) flags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (has_flag(stages, RAL::ShaderStage::Fragment)) flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        // ... add other stages ...
        return flags;
    }

    inline VmaMemoryUsage ToVmaMemoryUsage(RAL::MemoryUsage usage) {
        switch (usage) {
            case RAL::MemoryUsage::DeviceLocal: return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            case RAL::MemoryUsage::HostVisible: return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        }
        return VMA_MEMORY_USAGE_AUTO;
    }

    inline VkBufferUsageFlags ToVulkanBufferUsage(RAL::BufferUsage usage) {
        VkBufferUsageFlags flags = 0;
        if (has_flag(usage, RAL::BufferUsage::VertexBuffer)) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (has_flag(usage, RAL::BufferUsage::IndexBuffer)) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (has_flag(usage, RAL::BufferUsage::UniformBuffer)) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (has_flag(usage, RAL::BufferUsage::TransferSrc)) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (has_flag(usage, RAL::BufferUsage::TransferDst)) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        return flags;
    }
}
