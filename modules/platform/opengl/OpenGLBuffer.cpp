// RDE_Project/modules/platform/opengl/OpenGLBuffer.cpp
#include "OpenGLBuffer.h"
#include "OpenGLDebug.h"
#include <glad/gl.h>
namespace RDE {
// --- VertexBuffer ---
    std::shared_ptr<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size) {
        return std::make_shared<OpenGLVertexBuffer>(vertices, size);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size) {
        return std::make_shared<OpenGLVertexBuffer>(size);
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t size) {
        glCreateBuffers(1, &m_renderer_id);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        GL_CHECK_ERROR();
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
        glCreateBuffers(1, &m_renderer_id);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
        // Use GL_DYNAMIC_DRAW because we will be updating this buffer frequently
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        GL_CHECK_ERROR();
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        if (m_renderer_id) {
            glDeleteBuffers(1, &m_renderer_id);
            GL_CHECK_ERROR();
        }
    }

    void OpenGLVertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLVertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR();
    }

    void OpenGLVertexBuffer::set_data(const void *data, uint32_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
        // glBufferSubData is more efficient for updating than glBufferData
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
        GL_CHECK_ERROR();
    }

// --- IndexBuffer ---
    std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t count) {
        return std::make_shared<OpenGLIndexBuffer>(indices, count);
    }

    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t count) : m_count(count) {
        glCreateBuffers(1, &m_renderer_id);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        GL_CHECK_ERROR();
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        if (m_renderer_id) {
            glDeleteBuffers(1, &m_renderer_id);
            GL_CHECK_ERROR();
        }
    }

    void OpenGLIndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLIndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR();
    }
}