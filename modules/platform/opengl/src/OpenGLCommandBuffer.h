#pragma once

#include "ICommandBuffer.h"
#include "OpenGLDevice.h" // We need the concrete device to look up resources

namespace RDE {

    class OpenGLCommandBuffer : public ICommandBuffer {
    public:
        // The command buffer needs access to the device to resolve handles into GL IDs.
        explicit OpenGLCommandBuffer(OpenGLDevice *device);

        // --- Command Buffer Lifecycle ---
        void begin();

        void end();

        // --- ICommandBuffer Interface Implementation ---
        void bind_pipeline(GpuPipelineHandle pipeline) override;

        void bind_vertex_buffer(GpuBufferHandle buffer, uint32_t binding = 0, uint64_t offset = 0) override;

        void bind_index_buffer(GpuBufferHandle buffer, uint64_t offset = 0) override;

        void bind_texture(GpuTextureHandle texture, uint32_t slot) override;

        void bind_uniform_buffer(GpuBufferHandle buffer, uint32_t slot, size_t offset, size_t size) override;

        void push_constants(ShaderStage stage, const void *data, size_t size, uint32_t offset) override;

        void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                          uint32_t first_instance) override;

        void
        draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;

        void set_viewport(float x, float y, float width, float height) override;

    private:
        OpenGLDevice *m_device;

        // --- Internal State Cache ---
        // Caching the bound state to avoid redundant GL calls.
        GpuPipelineHandle m_bound_pipeline;
        unsigned int m_bound_vao = 0; // OpenGL directly binds VAOs, not separate VBs/IBs in the core profile command stream.
    };

}