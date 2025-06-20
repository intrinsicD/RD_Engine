// RDE_Project/modules/platform/opengl/OpenGLVertexArray.cpp
#include "OpenGLVertexArray.h"
#include "OpenGLDebug.h"

#include <glad/gl.h>

namespace RDE {
    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Float2:
                return GL_FLOAT;
            case ShaderDataType::Float3:
                return GL_FLOAT;
            case ShaderDataType::Float4:
                return GL_FLOAT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
            case ShaderDataType::None:
                break;
            default:
                RDE_CORE_ASSERT(false, "Unknown ShaderDataTypeToOpenGLBaseType::ShaderDataType!");
                return GL_NONE; // Return an invalid type if unknown
        }
        return 0;
    }

    std::shared_ptr<VertexArray> VertexArray::Create() {
        return std::make_shared<OpenGLVertexArray>();
    }

    OpenGLVertexArray::OpenGLVertexArray() {
        glCreateVertexArrays(1, &m_renderer_id);
        GL_CHECK_ERROR();
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLVertexArray::bind() const {
        glBindVertexArray(m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLVertexArray::unbind() const {
        glBindVertexArray(0);
        GL_CHECK_ERROR();
    }

    void OpenGLVertexArray::add_vertex_buffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) {
        RDE_CORE_ASSERT(vertexBuffer->get_layout().get_elements().size(), "Vertex Buffer has no layout!");
        glBindVertexArray(m_renderer_id);
        GL_CHECK_ERROR();
        vertexBuffer->bind();

        const auto &layout = vertexBuffer->get_layout();
        for (const auto &element: layout) {
            glEnableVertexAttribArray(m_vertex_buffer_index);
            GL_CHECK_ERROR();
            glVertexAttribPointer(m_vertex_buffer_index,
                                  element.GetComponentCount(),
                                  ShaderDataTypeToOpenGLBaseType(element.type),
                                  element.normalized ? GL_TRUE : GL_FALSE,
                                  layout.get_stride(),
                                  (const void *) element.offset);
            GL_CHECK_ERROR();
            m_vertex_buffer_index++;
        }
        m_vertex_buffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::set_index_buffer(const std::shared_ptr<IndexBuffer> &indexBuffer) {
        glBindVertexArray(m_renderer_id);
        GL_CHECK_ERROR();
        indexBuffer->bind();
        m_index_buffer = indexBuffer;
    }
}