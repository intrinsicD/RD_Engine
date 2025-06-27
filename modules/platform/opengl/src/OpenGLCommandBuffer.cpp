#include "OpenGLCommandBuffer.h"
#include "OpenGLDebug.h"

#include <iostream>

namespace RDE {

    // --- Helper function to translate our enums to GL enums ---
    GLenum ToGLTopology(PrimitiveTopologyType topology) {
        switch (topology) {
            case PrimitiveTopologyType::Points:         return GL_POINTS;
            case PrimitiveTopologyType::Lines:          return GL_LINES;
            case PrimitiveTopologyType::LineStrip:      return GL_LINE_STRIP;
            case PrimitiveTopologyType::Triangles:      return GL_TRIANGLES;
            case PrimitiveTopologyType::TriangleStrip:  return GL_TRIANGLE_STRIP;
            case PrimitiveTopologyType::TriangleFan:    return GL_TRIANGLE_FAN;
        }
        return GL_TRIANGLES; // Default
    }

    // --- Constructor & Lifecycle ---
    OpenGLCommandBuffer::OpenGLCommandBuffer(OpenGLDevice* device) : m_device(device) {
        if (!m_device) {
            throw std::runtime_error("OpenGLCommandBuffer must be initialized with a valid device!");
        }
    }

    void OpenGLCommandBuffer::begin() {
        // Reset the cache at the start of a new command recording session.
        m_bound_pipeline = {};
        m_bound_vao = 0;
    }

    void OpenGLCommandBuffer::end() {
        // Unbind things for safety/cleanup if desired.
        glBindVertexArray(0);
        GL_CHECK_ERROR();
    }

    // --- Interface Implementation ---

    void OpenGLCommandBuffer::bind_pipeline(GpuPipelineHandle pipeline_handle) {
        if (m_bound_pipeline.id == pipeline_handle.id) {
            return; // This pipeline is already bound, do nothing.
        }

        const auto& pipeline_gl = m_device->get_pipeline(pipeline_handle);
        const auto& desc = pipeline_gl.desc;

        // 1. Bind Program
        const auto& program_gl = m_device->get_program(desc.program);
        glUseProgram(program_gl.id);
        GL_CHECK_ERROR();

        // 2. Set Rasterizer State
        glPolygonMode(GL_FRONT_AND_BACK, desc.wireframe ? GL_LINE : GL_FILL);
        GL_CHECK_ERROR();

        // 3. Set Depth/Stencil State
        if (desc.depth_test_enable) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        GL_CHECK_ERROR();
        glDepthMask(desc.depth_write_enable ? GL_TRUE : GL_FALSE);
        GL_CHECK_ERROR();
        // glDepthFunc(...); // You would add a DepthCompareOp to your desc for this

        // 4. Set Vertex Layout on the currently bound VAO (this is a bit tricky)
        // This assumes a VAO is already bound via `bind_vertex_buffer`.
        // The vertex layout from the pipeline is applied to the bound VAO.
        if (m_bound_vao != 0) {
            for (const auto& attr : desc.vertex_layout.attributes) {
                glEnableVertexArrayAttrib(m_bound_vao, attr.location);
                // Note: The format conversion here would be more complex for a full engine.
                // This is a simplified example. We'd need a helper ToGLVertexFormat.
                glVertexArrayAttribFormat(m_bound_vao, attr.location, 3, GL_FLOAT, GL_FALSE, attr.offset);
                glVertexArrayAttribBinding(m_bound_vao, attr.location, 0); // Assuming binding point 0
            }
            glVertexArrayBindingDivisor(m_bound_vao, 0, 0); // Not instanced
            GL_CHECK_ERROR();
        }

        m_bound_pipeline = pipeline_handle;
    }

    // In OpenGL, binding vertex/index buffers is part of the VAO state.
    // Our create_geometry call already created a VAO and bound the buffers to it.
    // So, bind_vertex_buffer and bind_index_buffer effectively just mean "bind this VAO".
    void OpenGLCommandBuffer::bind_vertex_buffer(GpuBufferHandle buffer, uint32_t binding, uint64_t offset) {
        // This command is a bit of a lie in our current abstraction. We don't bind vertex buffers directly.
        // We look up the VAO that USES this vertex buffer.
        // A better abstraction would be bind_geometry(GpuGeometryHandle). Let's use that logic here.

        // This is a placeholder. You'd need a map in your device from GpuBufferHandle to the GpuGeometryHandles that use it.
        // For now, let's assume one VAO per vertex buffer for simplicity.
        // Find the geometry that uses this buffer. This is a weakness in the abstraction for GL.
        // We'll bind the first VAO we find.
        for (const auto& pair : m_device->get_all_geometries()) { // Assumes get_all_geometries() exists
            const auto& geom_desc = m_device->get_geometry_desc(pair.first); // Assumes get_geometry_desc() exists
            if (geom_desc.vertex_buffer.id == buffer.id) {
                GLuint vao_id = pair.second;
                if (m_bound_vao != vao_id) {
                    glBindVertexArray(vao_id);
                    GL_CHECK_ERROR();
                    m_bound_vao = vao_id;
                }
                return;
            }
        }
    }

    void OpenGLCommandBuffer::bind_index_buffer(GpuBufferHandle buffer, uint64_t offset) {
        // This is handled by the VAO binding in bind_vertex_buffer. We can make this a no-op.
    }

    void OpenGLCommandBuffer::bind_texture(GpuTextureHandle texture_handle, uint32_t slot) {
        const auto& texture_gl = m_device->get_texture(texture_handle);
        glBindTextureUnit(slot, texture_gl.id);
        GL_CHECK_ERROR();
    }

    void OpenGLCommandBuffer::bind_uniform_buffer(GpuBufferHandle buffer_handle, uint32_t slot, size_t offset, size_t size) {
        const auto& buffer_gl = m_device->get_buffer(buffer_handle);
        // If size is 0, bind the whole buffer
        if (size == 0) {
            glBindBufferBase(GL_UNIFORM_BUFFER, slot, buffer_gl.id);
            GL_CHECK_ERROR();
        } else {
            glBindBufferRange(GL_UNIFORM_BUFFER, slot, buffer_gl.id, offset, size);
            GL_CHECK_ERROR();
        }
    }

    void OpenGLCommandBuffer::push_constants(ShaderStage stage, const void* data, size_t size, uint32_t offset) {
        // OpenGL doesn't have a direct equivalent to "push constants".
        // The common emulation is to use a dedicated UBO that you update every frame.
        // A simpler, but slower, way is to use glUniform* calls.
        // This requires shader reflection to get uniform locations, which is complex.
        // For now, we'll leave this unimplemented as it highlights a key difference
        // between GL and modern APIs.
        std::cerr << "Warning: push_constants is not efficiently implemented for OpenGL." << std::endl;
    }

    void OpenGLCommandBuffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
        glDrawElementsInstancedBaseVertexBaseInstance(
                ToGLTopology(m_device->get_pipeline(m_bound_pipeline).desc.topology),
                index_count,
                GL_UNSIGNED_INT, // Assumes 32-bit indices. Add to GeometryDesc for 16-bit.
                (void*)(sizeof(uint32_t) * first_index),
                instance_count,
                vertex_offset,
                first_instance
        );
        GL_CHECK_ERROR();
    }

    void OpenGLCommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
        glDrawArraysInstancedBaseInstance(
                ToGLTopology(m_device->get_pipeline(m_bound_pipeline).desc.topology),
                first_vertex,
                vertex_count,
                instance_count,
                first_instance
        );
        GL_CHECK_ERROR();
    }

    void OpenGLCommandBuffer::set_viewport(float x, float y, float width, float height) {
        glViewport(x, y, width, height);
        GL_CHECK_ERROR();
    }
}