// ral/Resources.h
#pragma once

#include "Common.h" // For handles
#include <vector> // For handles
#include <string> // For handles

namespace RAL {
    // --- Resource Usage Flags ---
    enum class BufferUsage : uint32_t {
        None = 0,
        VertexBuffer  = 1 << 0,
        IndexBuffer   = 1 << 1,
        UniformBuffer = 1 << 2,
        StorageBuffer = 1 << 3,
        TransferSrc   = 1 << 4,
        TransferDst   = 1 << 5,
    };
    ENABLE_ENUM_FLAG_OPERATORS(BufferUsage)

    enum class TextureUsage : uint32_t {
        None = 0,
        Sampled                = 1 << 0,
        Storage                = 1 << 1,
        ColorAttachment        = 1 << 2,
        DepthStencilAttachment = 1 << 3,
        TransferSrc            = 1 << 4,
        TransferDst            = 1 << 5,
    };
    ENABLE_ENUM_FLAG_OPERATORS(TextureUsage)

    enum class MemoryUsage {
        DeviceLocal, // GPU only, fastest access
        HostVisible, // CPU visible, for frequent updates (e.g. UBOs)
        // No need for separate Coherent/Cached here, the backend can decide
        // the best flags based on this high-level intent.
    };

    enum class ShaderStage {
        Vertex, Fragment, Compute, // Core stages
        Geometry, TessellationControl, TessellationEvaluation, // Optional stages
        Task, Mesh // Modern mesh pipeline stages
    };

    // --- Resource Description Structs ---

    struct BufferDescription {
        uint64_t size = 0;
        BufferUsage usage;
        MemoryUsage memoryUsage;
        const void* initialData = nullptr; // Add this for easier creation
    };

    struct TextureDescription {
        uint32_t width = 1, height = 1, depth = 1;
        uint32_t mipLevels = 1;
        Format format; // Use the single, unified Format enum
        TextureUsage usage;
    };

    struct ShaderDescription {
        std::string filePath;
        ShaderStage stage;
        std::string entryPoint = "main";
    };

    struct VertexInputAttribute {
        uint32_t location;
        uint32_t binding;
        Format format;
        uint32_t offset;
    };

    struct VertexInputBinding {
        uint32_t binding;
        uint32_t stride;
    };

    struct PipelineDescription {
        ShaderHandle vertexShader;
        ShaderHandle fragmentShader;
        std::vector<VertexInputBinding> vertexBindings;
        std::vector<VertexInputAttribute> vertexAttributes;
    };

    struct SwapchainDescription {
        void* nativeWindowHandle = nullptr;
        bool vsync = true;
    };
}