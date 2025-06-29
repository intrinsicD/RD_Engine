#pragma once

#include "Common.h"
#include "DescriptorSet.h"

#include <memory>
#include <vector>

namespace RAL {
    // Forward declarations
    class CommandBuffer;
    struct BufferDescription;
    struct TextureDescription;
    struct PipelineDescription;

    class Device {
    public:
        virtual ~Device() = default;

        // --- Resource Factories ---
        virtual BufferHandle create_buffer(const BufferDescription &desc) = 0;

        virtual TextureHandle create_texture(const TextureDescription &desc) = 0;

        virtual PipelineHandle create_pipeline(const PipelineDescription &desc) = 0;

        // ... create_sampler, create_descriptor_set_layout, etc.

        // Creates the "template".
        virtual DescriptorSetLayoutHandle create_descriptor_set_layout(const DescriptorSetLayoutDescription &desc) = 0;

        virtual void destroy_descriptor_set_layout(DescriptorSetLayoutHandle handle) = 0;

        // Allocates one or more "instances" from a layout. Often done from a pool.
        virtual std::vector<DescriptorSetHandle> allocate_descriptor_sets(
            DescriptorSetLayoutHandle layout, uint32_t count) = 0;

        // virtual void free_descriptor_sets(...) = 0;

        // Updates the contents of descriptor sets to point to actual resources.
        virtual void update_descriptor_sets(const std::vector<WriteDescriptorSet> &writes) = 0;

        // --- Resource Destruction ---
        virtual void destroy_buffer(BufferHandle handle) = 0;

        virtual void destroy_texture(TextureHandle handle) = 0;

        virtual void destroy_pipeline(PipelineHandle handle) = 0;

        // ...

        // --- Command Buffer Management ---

        // Creates a command buffer object.
        virtual std::unique_ptr<CommandBuffer> create_command_buffer() = 0;

        // The most important execution function.
        // Takes one or more completed command buffers and submits them to the GPU.
        virtual void submit_command_buffers(const std::vector<CommandBuffer *> &command_buffers) = 0;

        // --- Other Operations ---

        // Waits for all pending GPU work to complete.
        virtual void wait_idle() = 0;
    };
} // namespace RAL
