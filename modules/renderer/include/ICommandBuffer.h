#pragma once

#include "RendererTypes.h"

namespace RDE {
    class ICommandBuffer {
    public:
        virtual void bind_pipeline(GpuPipelineHandle pipeline) = 0;

        // Binds a vertex buffer to a specific binding point (usually 0)
        virtual void bind_vertex_buffer(GpuBufferHandle buffer, uint32_t binding = 0, uint64_t offset = 0) = 0;

        // Binds the index buffer for indexed drawing
        virtual void bind_index_buffer(GpuBufferHandle buffer, uint64_t offset = 0) = 0;

        // Binds a texture to a specific texture unit/slot/binding
        virtual void bind_texture(GpuTextureHandle texture, uint32_t slot) = 0;

        // Binds a uniform buffer to a specific binding point
        virtual void bind_uniform_buffer(GpuBufferHandle buffer, uint32_t slot, size_t offset = 0, size_t size = 0) = 0;

        // Updates a small amount of uniform data without a buffer (maps to push constants in Vulkan)
        virtual void push_constants(ShaderStage stage, const void* data, size_t size, uint32_t offset = 0) = 0;

        // The full draw call signatures
        virtual void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) = 0;
        virtual void draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;

        virtual void set_viewport(float x, float y, float width, float height) = 0;
    };
}