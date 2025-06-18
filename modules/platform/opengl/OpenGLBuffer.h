// RDE_Project/modules/platform/opengl/OpenGLBuffer.h
#pragma once

#include "Renderer/Buffer.h"

class OpenGLVertexBuffer : public VertexBuffer {
public:
    OpenGLVertexBuffer(float *vertices, uint32_t size);

    OpenGLVertexBuffer(uint32_t size);

    ~OpenGLVertexBuffer() override;

    void Bind() const override;

    void Unbind() const override;

    const BufferLayout &GetLayout() const override { return m_layout; }

    void SetLayout(const BufferLayout &layout) override { m_layout = layout; }

    void SetData(const void* data, uint32_t size) override;

private:
    uint32_t m_renderer_id;
    BufferLayout m_layout;
};

class OpenGLIndexBuffer : public IndexBuffer {
public:
    OpenGLIndexBuffer(uint32_t *indices, uint32_t count);

    ~OpenGLIndexBuffer() override;

    void Bind() const override;

    void Unbind() const override;

    uint32_t GetCount() const override { return m_count; }

private:
    uint32_t m_renderer_id;
    uint32_t m_count;
};