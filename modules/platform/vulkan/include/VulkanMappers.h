//vulkan/VulkanMappers.h
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
            case RAL::Format::R8_UNORM:
                return VK_FORMAT_R8_UNORM;
            case RAL::Format::R8G8_UNORM:
                return VK_FORMAT_R8G8_UNORM;
            case RAL::Format::R8G8B8A8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case RAL::Format::B8G8R8A8_UNORM:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case RAL::Format::R8_SRGB:
                return VK_FORMAT_R8_SRGB;
            case RAL::Format::R8G8_SRGB:
                return VK_FORMAT_R8G8_SRGB;
            case RAL::Format::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case RAL::Format::B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
                // 16-bit
            case RAL::Format::R16_SFLOAT:
                return VK_FORMAT_R16_SFLOAT;
            case RAL::Format::R16G16_SFLOAT:
                return VK_FORMAT_R16G16_SFLOAT;
            case RAL::Format::R16G16B16A16_SFLOAT:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
                // 32-bit
            case RAL::Format::R32_SFLOAT:
                return VK_FORMAT_R32_SFLOAT;
            case RAL::Format::R32G32_SFLOAT:
                return VK_FORMAT_R32G32_SFLOAT;
            case RAL::Format::R32G32B32_SFLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case RAL::Format::R32G32B32A32_SFLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case RAL::Format::R32_UINT:
                return VK_FORMAT_R32_UINT;
            case RAL::Format::R32G32_UINT:
                return VK_FORMAT_R32G32_UINT;
            case RAL::Format::R32G32B32_UINT:
                return VK_FORMAT_R32G32B32_UINT;
            case RAL::Format::R32G32B32A32_UINT:
                return VK_FORMAT_R32G32B32A32_UINT;
                // Depth
            case RAL::Format::D32_SFLOAT:
                return VK_FORMAT_D32_SFLOAT;
            case RAL::Format::D24_UNORM_S8_UINT:
                return VK_FORMAT_D24_UNORM_S8_UINT;
            case RAL::Format::D32_SFLOAT_S8_UINT:
                return VK_FORMAT_D32_SFLOAT_S8_UINT;
                // Block Compression
            case RAL::Format::BC1_RGB_UNORM:
                return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case RAL::Format::BC3_UNORM:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case RAL::Format::BC7_UNORM:
                return VK_FORMAT_BC7_UNORM_BLOCK;

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
            case RAL::SamplerAddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case RAL::SamplerAddressMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case RAL::SamplerAddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case RAL::SamplerAddressMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    inline VkDescriptorType ToVulkanDescriptorType(RAL::DescriptorType type) {
        switch (type) {
            case RAL::DescriptorType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case RAL::DescriptorType::StorageBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case RAL::DescriptorType::SampledImage:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case RAL::DescriptorType::StorageImage:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case RAL::DescriptorType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            case RAL::DescriptorType::CombinedImageSampler:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    inline VkShaderStageFlags ToVulkanShaderStageFlags(RAL::ShaderStage stages) {
        VkShaderStageFlags flags = 0;
        if (has_flag(stages, RAL::ShaderStage::Vertex)) flags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (has_flag(stages, RAL::ShaderStage::Fragment)) flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (has_flag(stages, RAL::ShaderStage::Compute)) flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        if (has_flag(stages, RAL::ShaderStage::Geometry)) flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if (has_flag(stages, RAL::ShaderStage::TessellationControl)) flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (has_flag(stages, RAL::ShaderStage::TessellationEvaluation))
            flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if (has_flag(stages, RAL::ShaderStage::RayTracing))
            flags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
                     VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR |
                     VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        if (has_flag(stages, RAL::ShaderStage::Task)) flags |= VK_SHADER_STAGE_TASK_BIT_EXT;
        if (has_flag(stages, RAL::ShaderStage::Mesh)) flags |= VK_SHADER_STAGE_MESH_BIT_EXT;

        // ... add other stages ...
        return flags;
    }

    inline VmaMemoryUsage ToVmaMemoryUsage(RAL::MemoryUsage usage) {
        switch (usage) {
            case RAL::MemoryUsage::DeviceLocal:
                return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            case RAL::MemoryUsage::HostVisible:
                return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case RAL::MemoryUsage::CPU_To_GPU:
                return VMA_MEMORY_USAGE_CPU_TO_GPU;
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

    inline VkImageUsageFlags ToVulkanImageUsage(RAL::TextureUsage usage) {
        VkImageUsageFlags flags = 0;
        if (has_flag(usage, RAL::TextureUsage::Sampled)) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (has_flag(usage, RAL::TextureUsage::Storage)) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (has_flag(usage, RAL::TextureUsage::ColorAttachment)) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (has_flag(usage, RAL::TextureUsage::DepthStencilAttachment))
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (has_flag(usage, RAL::TextureUsage::TransferSrc)) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (has_flag(usage, RAL::TextureUsage::TransferDst)) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return flags;
    }

    inline VkBlendFactor ToVulkanBlendFactor(RAL::BlendFactor factor) {
        switch (factor) {
            case RAL::BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case RAL::BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case RAL::BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case RAL::BlendFactor::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case RAL::BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case RAL::BlendFactor::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case RAL::BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case RAL::BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case RAL::BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case RAL::BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                // Add other factors as needed
        }
        return VK_BLEND_FACTOR_ZERO; // Default fallback
    }

    inline VkBlendOp ToVulkanBlendOp(RAL::BlendOp op) {
        switch (op) {
            case RAL::BlendOp::Add:
                return VK_BLEND_OP_ADD;
            case RAL::BlendOp::Subtract:
                return VK_BLEND_OP_SUBTRACT;
            case RAL::BlendOp::ReverseSubtract:
                return VK_BLEND_OP_REVERSE_SUBTRACT;
            case RAL::BlendOp::Min:
                return VK_BLEND_OP_MIN;
            case RAL::BlendOp::Max:
                return VK_BLEND_OP_MAX;
        }
        return VK_BLEND_OP_ADD; // Default fallback
    }

    inline VkPolygonMode ToVulkanPolygonMode(RAL::PolygonMode mode) {
        switch (mode) {
            case RAL::PolygonMode::Fill:
                return VK_POLYGON_MODE_FILL;
            case RAL::PolygonMode::Line:
                return VK_POLYGON_MODE_LINE;
            case RAL::PolygonMode::Point:
                return VK_POLYGON_MODE_POINT;
        }
        return VK_POLYGON_MODE_FILL; // Default fallback
    }

    inline VkCullModeFlags ToVulkanCullMode(RAL::CullMode mode) {
        switch (mode) {
            case RAL::CullMode::None:
                return VK_CULL_MODE_NONE;
            case RAL::CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case RAL::CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            case RAL::CullMode::FrontAndBack:
                return VK_CULL_MODE_FRONT_AND_BACK;
        }
        return VK_CULL_MODE_NONE; // Default fallback
    }

    inline VkFrontFace ToVulkanFrontFace(RAL::FrontFace frontFace) {
        switch (frontFace) {
            case RAL::FrontFace::Clockwise:
                return VK_FRONT_FACE_CLOCKWISE;
            case RAL::FrontFace::CounterClockwise:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }
        return VK_FRONT_FACE_COUNTER_CLOCKWISE; // Default fallback
    }

    inline std::string ToString(VkResult result) {
        switch (result) {
            case VK_SUCCESS:
                return "VK_SUCCESS";
            case VK_NOT_READY:
                return "VK_NOT_READY";
            case VK_TIMEOUT:
                return "VK_TIMEOUT";
            case VK_EVENT_SET:
                return "VK_EVENT_SET";
            case VK_EVENT_RESET:
                return "VK_EVENT_RESET";
            case VK_INCOMPLETE:
                return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:
                return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:
                return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_UNKNOWN:
                return "VK_ERROR_UNKNOWN";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case VK_ERROR_FRAGMENTATION:
                return "VK_ERROR_FRAGMENTATION";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
                return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case VK_PIPELINE_COMPILE_REQUIRED:
                return "VK_PIPELINE_COMPILE_REQUIRED";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:
                return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:
                return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_VALIDATION_FAILED_EXT:
                return "VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_INVALID_SHADER_NV:
                return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
                return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
                return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
                return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
                return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
                return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
                return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
                return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_NOT_PERMITTED_KHR:
                return "VK_ERROR_NOT_PERMITTED_KHR";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case VK_THREAD_IDLE_KHR:
                return "VK_THREAD_IDLE_KHR";
            case VK_THREAD_DONE_KHR:
                return "VK_THREAD_DONE_KHR";
            case VK_OPERATION_DEFERRED_KHR:
                return "VK_OPERATION_DEFERRED_KHR";
            case VK_OPERATION_NOT_DEFERRED_KHR:
                return "VK_OPERATION_NOT_DEFERRED_KHR";
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
                return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
                return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
                return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            case VK_PIPELINE_BINARY_MISSING_KHR:
                return "VK_PIPELINE_BINARY_MISSING_KHR";
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
                return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
            case VK_RESULT_MAX_ENUM:
                return "VK_RESULT_MAX_ENUM";
            default:
                return "Unknown VkResult value!";
        }
    }
}
