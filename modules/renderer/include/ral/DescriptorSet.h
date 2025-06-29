#pragma once

#include "Common.h"
#include <vector>
#include <variant>

namespace RAL {

    // Describes the type of a single resource binding.
    enum class DescriptorType {
        UNIFORM_BUFFER,
        SAMPLED_TEXTURE,
        SAMPLER,
        STORAGE_BUFFER,
        STORAGE_IMAGE,
        // ... and others
    };

    // Describes which shader stages can access a binding.
    // Can be bitwise OR'd together (e.g., VERTEX | FRAGMENT).
    enum ShaderStageFlags {
        NONE = 0,
        VERTEX = 1 << 0,
        FRAGMENT = 1 << 1,
        COMPUTE = 1 << 2,
        // ... other stages
        ALL_GRAPHICS = VERTEX | FRAGMENT,
        ALL = ~0
    };

    ENABLE_ENUM_FLAG_OPERATORS(ShaderStageFlags)

    // Describes a single binding point within a DescriptorSetLayout.
    // e.g., "binding = 0, type = SAMPLED_TEXTURE, stage = FRAGMENT"
    struct DescriptorSetLayoutBinding {
        uint32_t binding_index = 0;
        DescriptorType descriptor_type = DescriptorType::UNIFORM_BUFFER;
        uint32_t descriptor_count = 1; // For arrays of resources
        ShaderStageFlags stage_flags = ShaderStageFlags::NONE;
    };

    // The "template" for a descriptor set.
    struct DescriptorSetLayoutDescription {
        std::vector<DescriptorSetLayoutBinding> bindings;
    };


    // --- Now for the actual DescriptorSet data ---

    // A pointer to a specific buffer resource for updating a set.
    struct BufferDescriptorInfo {
        BufferHandle handle;
        uint64_t offset = 0;
        uint64_t range = -1; // WHOLE_BUFFER
    };

    // A pointer to a specific texture resource for updating a set.
    struct TextureDescriptorInfo {
        TextureHandle handle;
        // SamplerHandle sampler; // Could be combined or separate
        // TextureLayout expected_layout;
    };

    // Describes an update operation for a single binding in a DescriptorSet.
    struct WriteDescriptorSet {
        uint32_t dst_binding_index = 0;
        DescriptorType descriptor_type = DescriptorType::UNIFORM_BUFFER;

        // Use a union or std::variant for the actual data to avoid storing pointers for all types.
        std::variant<
            std::vector<BufferDescriptorInfo>,
            std::vector<TextureDescriptorInfo>
        > resource_infos;
    };

} // namespace RAL