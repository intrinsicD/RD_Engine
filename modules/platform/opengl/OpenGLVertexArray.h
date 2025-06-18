// RDE_Project/modules/platform/opengl/OpenGLVertexArray.h
#pragma once

#include "Renderer/VertexArray.h"
namespace RDE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        virtual ~OpenGLVertexArray();

        void Bind() const override;

        void Unbind() const override;

        void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;

        void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

        const std::vector<std::shared_ptr<VertexBuffer>> &
        GetVertexBuffers() const override { return m_vertex_buffers; };

        const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const override { return m_index_buffer; };
    private:
        uint32_t m_renderer_id;
        uint32_t m_vertex_buffer_index = 0;
        std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
        std::shared_ptr<IndexBuffer> m_index_buffer;
    };
}