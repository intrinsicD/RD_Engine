// RDE_Project/modules/renderer/include/Renderer/Buffer.h

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "Core/Log.h"

enum class ShaderDataType {
    None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
            return 4;
        case ShaderDataType::Float2:
            return 4 * 2;
        case ShaderDataType::Float3:
            return 4 * 3;
        case ShaderDataType::Float4:
            return 4 * 4;
        case ShaderDataType::Mat3:
            return 4 * 3 * 3;
        case ShaderDataType::Mat4:
            return 4 * 4 * 4;
        case ShaderDataType::Int:
            return 4;
        case ShaderDataType::Int2:
            return 4 * 2;
        case ShaderDataType::Int3:
            return 4 * 3;
        case ShaderDataType::Int4:
            return 4 * 4;
        case ShaderDataType::Bool:
            return 1;
        default:
            RDE_CORE_ASSERT(false, "Unknown ShaderDataTypeSize::ShaderDataType!");
            return 0;
    }
}

struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    size_t offset;
    bool normalized;

    BufferElement(ShaderDataType p_type, const std::string &p_name, bool p_normalized = false)
            : name(p_name), type(p_type), size(ShaderDataTypeSize(p_type)), offset(0), normalized(p_normalized) {}

    uint32_t GetComponentCount() const {
        switch (type) {
            case ShaderDataType::Float:
                return 1;
            case ShaderDataType::Float2:
                return 2;
            case ShaderDataType::Float3:
                return 3;
            case ShaderDataType::Float4:
                return 4;
            default:
                RDE_CORE_ASSERT(false, "Unknown GetComponentCount::ShaderDataType!");
                return 0;
        }
    }
};

class BufferLayout {
public:
    BufferLayout() = default; // Add a default constructor.

    BufferLayout(const std::initializer_list<BufferElement> &elements)
            : m_elements(elements) {
        CalculateOffsetsAndStride();
    }

    uint32_t GetStride() const { return m_stride; }

    const std::vector<BufferElement> &GetElements() const { return m_elements; }

    std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }

    std::vector<BufferElement>::iterator end() { return m_elements.end(); }

    std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }

    std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

private:
    void CalculateOffsetsAndStride() {
        size_t offset = 0;
        m_stride = 0;
        for (auto &element: m_elements) {
            element.offset = offset;
            offset += element.size;
            m_stride += element.size;
        }
    }

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;
};


class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    virtual const BufferLayout &GetLayout() const = 0;

    virtual void SetLayout(const BufferLayout &layout) = 0;

    virtual void SetData(const void* data, uint32_t size) = 0;

    static std::shared_ptr<VertexBuffer> Create(float *vertices, uint32_t size);

    static std::shared_ptr<VertexBuffer> Create(uint32_t size);
};

class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    virtual uint32_t GetCount() const = 0;

    static std::shared_ptr<IndexBuffer> Create(uint32_t *indices, uint32_t count);
};