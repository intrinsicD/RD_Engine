//opengl/src/OpenGLCommandBuffer.cpp

#include "OpenGLCommandBuffer.h"
#include "OpenGLMappers.h"

namespace RDE{
    OpenGLCommandBuffer::OpenGLCommandBuffer(OpenGLDevice *device)
            : m_device(device) {}

    void OpenGLCommandBuffer::begin() {
        assert(!m_recording);
        m_recording = true;
    }

    void OpenGLCommandBuffer::end() {
        assert(m_recording);
        if (m_inRenderPass) {
            end_render_pass();
        }
        m_recording = false;
    }

    void OpenGLCommandBuffer::begin_render_pass(const RAL::RenderPassDescription &desc) {
        assert(m_recording && !m_inRenderPass);
        m_inRenderPass = true;
        // Framebuffer management not yet implemented. Assume default framebuffer already bound.

        GLbitfield clearMask = 0;
        // Color clear: pick first attachment that requests clear to set glClearColor
        for (size_t i = 0; i < desc.colorAttachments.size(); ++i) {
            const auto &att = desc.colorAttachments[i];
            if (att.loadOp == RAL::LoadOp::Clear) {
                const auto &c = att.clearColor;
                glClearColor(c[0], c[1], c[2], c[3]);
                clearMask |= GL_COLOR_BUFFER_BIT;
                break; // One glClearColor call is sufficient
            }
        }
        // Depth/stencil clear if depth attachment texture handle valid and asks for clear
        if (desc.depthStencilAttachment.texture.is_valid() && desc.depthStencilAttachment.loadOp == RAL::LoadOp::Clear) {
            glClearDepth(desc.depthStencilAttachment.clearDepth);
            glClearStencil(desc.depthStencilAttachment.clearStencil);
            clearMask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        }
        if (clearMask) glClear(clearMask);
    }

    void OpenGLCommandBuffer::end_render_pass() {
        assert(m_inRenderPass);
        m_inRenderPass = false;
    }

    void OpenGLCommandBuffer::set_viewport(const RAL::Viewport &viewport) {
        glViewport(
                static_cast<GLint>(viewport.x),
                static_cast<GLint>(viewport.y),
                static_cast<GLsizei>(viewport.width),
                static_cast<GLsizei>(viewport.height)
        );
        glDepthRange(viewport.min_depth, viewport.max_depth);
    }

    void OpenGLCommandBuffer::set_scissor(const RAL::Rect2D &scissor) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(
                static_cast<GLint>(scissor.x),
                static_cast<GLint>(scissor.y),
                static_cast<GLsizei>(scissor.width),
                static_cast<GLsizei>(scissor.height)
        );
    }

    void OpenGLCommandBuffer::bind_pipeline(RAL::PipelineHandle /*pipeline*/) {
        // TODO: Implement real pipeline/program retrieval when OpenGLDevice stores programs.
    }

    void OpenGLCommandBuffer::pipeline_barrier(const RAL::ResourceBarrier &barrier) {
        // OpenGL coarse barrier: derive mask from dstAccess (most restrictive)
        GLbitfield mask = ToOpenGLMemoryBarrierMask(barrier.dstAccess);
        if (barrier.textureTransition.texture.is_valid()) {
            // Layout transitions are implicit; nothing else needed.
        }
        if (mask) glMemoryBarrier(mask);
    }

    void OpenGLCommandBuffer::bind_vertex_buffer(RAL::BufferHandle /*buffer*/, uint32_t /*binding*/) {
        // TODO: Implement VAO / glBindVertexBuffer handling.
    }

    void OpenGLCommandBuffer::bind_index_buffer(RAL::BufferHandle /*buffer*/, RAL::IndexType indexType) {
        m_indexTypeGL = ToOpenGLIndexType(indexType);
        // TODO: Store bound index buffer name after resolving handle.
    }

    void OpenGLCommandBuffer::bind_descriptor_set(RAL::PipelineHandle /*pipeline*/, RAL::DescriptorSetHandle /*set*/,
                                                  uint32_t /*setIndex*/) {
        // TODO: Implement descriptor emulation.
    }

    void OpenGLCommandBuffer::copy_buffer(RAL::BufferHandle /*src*/, RAL::BufferHandle /*dst*/,
                                          uint64_t /*size*/, uint64_t /*srcOffset*/, uint64_t /*dstOffset*/) {
        // TODO: Implement when OpenGL buffers are tracked.
    }

    void OpenGLCommandBuffer::copy_buffer_to_texture(RAL::BufferHandle /*src*/,
                                                     RAL::TextureHandle /*dst*/,
                                                     const std::vector<RAL::BufferTextureCopy> &/*regions*/) {
        // TODO: Implement PBO assisted uploads.
    }

    void OpenGLCommandBuffer::push_constants(RAL::PipelineHandle /*pipeline*/,
                                             RAL::ShaderStage /*stages*/,
                                             uint32_t /*offset*/,
                                             uint32_t /*size*/,
                                             const void * /*data*/) {
        // TODO: Implement UBO-backed push constants.
    }

    void OpenGLCommandBuffer::draw(uint32_t vertex_count,
                                   uint32_t instance_count,
                                   uint32_t first_vertex,
                                   uint32_t first_instance) {
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES,
                                          static_cast<GLint>(first_vertex),
                                          static_cast<GLsizei>(vertex_count),
                                          static_cast<GLsizei>(instance_count),
                                          static_cast<GLuint>(first_instance));
    }

    void OpenGLCommandBuffer::draw_indexed(uint32_t index_count,
                                           uint32_t instance_count,
                                           uint32_t first_index,
                                           int32_t vertex_offset,
                                           uint32_t first_instance) {
        const void *indexOffsetPtr = reinterpret_cast<const void*>(
                static_cast<uintptr_t>(first_index) * (m_indexTypeGL == GL_UNSIGNED_SHORT ? 2u : 4u));
        glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                      static_cast<GLsizei>(index_count),
                                                      m_indexTypeGL,
                                                      indexOffsetPtr,
                                                      static_cast<GLsizei>(instance_count),
                                                      vertex_offset,
                                                      first_instance);
    }

    void OpenGLCommandBuffer::dispatch(uint32_t groupCountX,
                                       uint32_t groupCountY,
                                       uint32_t groupCountZ) {
        glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    }
}