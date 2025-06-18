// RDE_Project/modules/platform/opengl/OpenGLVertexArray.h
#pragma once

#include "VertexArray.h"

namespace RDE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        ~OpenGLVertexArray() override;

        void bind() const override;

        void unbind() const override;

        void add_vertex_buffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;

        void set_index_buffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

        const std::vector<std::shared_ptr<VertexBuffer> > &
        get_vertex_buffers() const override { return m_vertex_buffers; };

        const std::shared_ptr<IndexBuffer> &get_index_buffer() const override { return m_index_buffer; };

    private:
        uint32_t m_renderer_id;
        uint32_t m_vertex_buffer_index = 0;
        std::vector<std::shared_ptr<VertexBuffer> > m_vertex_buffers;
        std::shared_ptr<IndexBuffer> m_index_buffer;
    };
}
