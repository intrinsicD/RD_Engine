//vulkan/OpenGLCommandBuffer.h
#pragma once

#include "ral/CommandBuffer.h"

#include <glad/gl.h>

namespace RDE {
    class OpenGLDevice; // Forward declare

    class OpenGLCommandBuffer : public RAL::CommandBuffer {
    public:
        explicit OpenGLCommandBuffer(OpenGLDevice *device);

        ~OpenGLCommandBuffer() override = default;

        void begin() override;

        void end() override;

        void begin_render_pass(const RAL::RenderPassDescription &desc) override;

        void end_render_pass() override;

        void set_viewport(const RAL::Viewport &viewport) override;

        void set_scissor(const RAL::Rect2D &scissor) override;

        void bind_pipeline(RAL::PipelineHandle pipeline) override;

        void pipeline_barrier(const RAL::ResourceBarrier &barrier) override;

        void bind_vertex_buffer(RAL::BufferHandle buffer, uint32_t binding) override;

        void bind_index_buffer(RAL::BufferHandle buffer, RAL::IndexType indexType) override;

        void bind_descriptor_set(RAL::PipelineHandle pipeline, RAL::DescriptorSetHandle set,
                                 uint32_t setIndex) override;

        void copy_buffer(RAL::BufferHandle src, RAL::BufferHandle dst, uint64_t size, uint64_t srcOffset = 0,
                         uint64_t dstOffset = 0) override;

        void copy_buffer_to_texture(RAL::BufferHandle src, RAL::TextureHandle dst, const std::vector<RAL::BufferTextureCopy> &regions) override;

        void push_constants(RAL::PipelineHandle pipeline, RAL::ShaderStage stages, uint32_t offset, uint32_t size,
                            const void *data) override;

        void draw(uint32_t vertex_count,
                  uint32_t instance_count,
                  uint32_t first_vertex,
                  uint32_t first_instance) override;

        void draw_indexed(uint32_t index_count,
                          uint32_t instance_count,
                          uint32_t first_index,
                          int32_t vertex_offset,
                          uint32_t first_instance) override;

        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        // --- OpenGL Specific ---


    private:
        OpenGLDevice *m_device;
        bool m_recording = false;
        bool m_inRenderPass = false;

        // Cached state (optional)
        GLuint m_boundProgram = 0;
        GLuint m_indexBuffer = 0;
        GLenum m_indexTypeGL = GL_UNSIGNED_INT;
    };
}
