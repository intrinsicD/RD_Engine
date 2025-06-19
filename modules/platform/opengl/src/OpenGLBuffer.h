// RDE_Project/modules/platform/opengl/OpenGLBuffer.h
#pragma once

#include "Buffer.h"

namespace RDE {
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float *vertices, uint32_t size);

        OpenGLVertexBuffer(uint32_t size);

        ~OpenGLVertexBuffer() override;

        void bind() const override;

        void unbind() const override;

        const BufferLayout &get_layout() const override { return m_layout; }

        void set_layout(const BufferLayout &layout) override { m_layout = layout; }

        void set_data(const void *data, uint32_t size) override;

    private:
        uint32_t m_renderer_id;
        BufferLayout m_layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(const uint32_t *indices, uint32_t count);

        ~OpenGLIndexBuffer() override;

        void bind() const override;

        void unbind() const override;

        uint32_t get_count() const override { return m_count; }

    private:
        uint32_t m_renderer_id;
        uint32_t m_count;
    };
}
