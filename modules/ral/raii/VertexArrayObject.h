// RDE_Project/modules/platform/opengl/OpenGLVertexArray.h
#pragma once

#include "Buffer.h"

#include <cstdint>
#include <vector>
#include <memory>


namespace RDE {
    class VertexArray {
    public:
        VertexArray();

        virtual ~VertexArray() = 0;

        virtual void bind() const = 0;

        virtual void unbind() const = 0;

        virtual void add_vertex_buffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;

        virtual void set_index_buffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

        const std::vector<std::shared_ptr<VertexBuffer> > &get_vertex_buffers() const override { return m_vertex_buffers; };

        const std::shared_ptr<IndexBuffer> &get_index_buffer() const override { return m_index_buffer; };

    private:
        uint32_t m_renderer_id;
        uint32_t m_vertex_buffer_index = 0;
        std::vector<std::shared_ptr<VertexBuffer> > m_vertex_buffers;
        std::shared_ptr<IndexBuffer> m_index_buffer;
    };
}
