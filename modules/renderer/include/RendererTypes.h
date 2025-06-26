#pragma once

#include <cstdint>
#include <vector>
#include <string>

// Forward declare to avoid including heavy headers
namespace RDE {
    class IWindow; // Assuming you have a window interface
}

namespace RDE {
    enum class GpuHandleType : uint8_t {
        Geometry, Texture, Material, Program, Buffer
    };

    enum class ShaderType : uint8_t {
        Vertex, Fragment, Geometry, Compute, TessellationControl, TessellationEvaluation
    };

    enum class BufferType : uint8_t {
        Uniform, Storage, Indirect
    };

    template<GpuHandleType T>
    class GpuHandle {
    public:
        uint64_t id = 0;

        bool is_valid() const { return id != 0; }

        // Add comparison operators for use in maps, etc.
        bool operator==(const GpuHandle &other) const {
            return id == other.id;
        }

        bool operator!=(const GpuHandle &other) const {
            return id != other.id;
        }
    };

    using GpuGeometryHandle = GpuHandle<GpuHandleType::Geometry>;
    using GpuTextureHandle = GpuHandle<GpuHandleType::Texture>;
    using GpuMaterialHandle = GpuHandle<GpuHandleType::Material>;
    using GpuProgramHandle = GpuHandle<GpuHandleType::Program>;
    using GpuBufferHandle = GpuHandle<GpuHandleType::Buffer>;

    // --- Core Enums ---

    enum class TextureFormat : uint8_t {
        // Add more as needed
        Unknown,
        R8,
        RG8,
        RGB8,
        RGBA8, // Common: 8 bits per channel, 4 channels
        R16F, // 16-bit float, 1 channel
        RG16F,
        RGBA16F, // Common for HDR render targets
        R32F, // 32-bit float, 1 channel
        RG32F,
        RGBA32F, // Common for high-precision data
        D24S8, // Common: 24-bit depth, 8-bit stencil
        D32F, // 32-bit float depth
    };

    enum class TextureUsage : uint8_t {
        // Use bit flags to allow multiple uses
        None = 0,
        Sampled = 1 << 0, // Can be bound as a texture in a shader
        RenderTarget = 1 << 1, // Can be used as a color attachment
        DepthStencil = 1 << 2, // Can be used as a depth/stencil attachment
        Storage = 1 << 3, // Can be used as a read/write storage image
        Upload = 1 << 4, // Can be a destination for data uploads
    };

    // Enable bitwise operations for this enum
    inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
        return static_cast<TextureUsage>(static_cast<int>(a) | static_cast<int>(b));
    }

    enum class BufferUsage : uint8_t {
        None = 0,
        VertexBuffer = 1 << 0,
        IndexBuffer = 1 << 1,
        UniformBuffer = 1 << 2,
        StorageBuffer = 1 << 3,
        Indirect = 1 << 4, // For indirect drawing commands
        Upload = 1 << 5, // For CPU-to-GPU data transfer (staging)
    };

    inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
        return static_cast<BufferUsage>(static_cast<int>(a) | static_cast<int>(b));
    }


    enum class ShaderStage : uint8_t {
        // Use bit flags
        None = 0,
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Compute = 1 << 2,
        Geometry = 1 << 3,
        TessControl = 1 << 4,
        TessEvaluation = 1 << 5,
    };

    inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
        return static_cast<ShaderStage>(static_cast<int>(a) | static_cast<int>(b));
    }

    enum class PrimitiveTopologyType : uint8_t {
        Points, Lines, LineStrip, Triangles, TriangleStrip, TriangleFan
    };

    // --- Descriptor Structs ---

    /**
     * @brief Describes a buffer resource.
     */
    struct BufferDesc {
        size_t size = 0;
        BufferUsage usage = BufferUsage::None;
        const void *initial_data = nullptr; // Optional: provide data on creation
        std::string debug_name;
    };

    /**
     * @brief Describes a single shader module.
     */
    struct ShaderModuleDesc {
        ShaderStage stage;
        // We use a vector of bytes to be API-agnostic.
        // For Vulkan, this is SPIR-V bytecode.
        // For OpenGL, this could be GLSL source code text.
        std::vector<char> source;
        std::string entry_point = "main";
    };

    /**
     * @brief Describes a full shader program/pipeline.
     */
    struct ProgramDesc {
        std::vector<ShaderModuleDesc> modules;
        std::string debug_name;
    };

    /**
     * @brief Describes a texture resource.
     */
    struct TextureDesc {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1; // For 3D textures
        uint32_t mip_levels = 1;
        TextureFormat format = TextureFormat::Unknown;
        TextureUsage usage = TextureUsage::None;
        const void *initial_data = nullptr; // Optional: provide data on creation
        std::string debug_name;
    };

    /**
     * @brief Describes a vertex buffer layout for a pipeline.
     */
    struct VertexAttributeDesc {
        uint32_t location; // e.g., layout(location = 0) in ...
        TextureFormat format;
        uint32_t offset;
    };

    struct VertexLayoutDesc {
        std::vector<VertexAttributeDesc> attributes;
        uint32_t stride = 0; // Size of one full vertex
    };

    /**
     * @brief Describes a full graphics pipeline state.
     * This is a big one. It combines shader programs with fixed-function state.
     * In Vulkan, this maps almost 1:1 to VkGraphicsPipelineCreateInfo.
     * In OpenGL, the backend will have to manage this state.
     */
    struct GraphicsPipelineDesc {
        GpuProgramHandle program; // Handle to the compiled shader program
        VertexLayoutDesc vertex_layout;
        PrimitiveTopologyType topology = PrimitiveTopologyType::Triangles;

        // --- Rasterizer State ---
        bool wireframe = false;
        // CullMode (Front, Back, None)
        // FrontFace (CW, CCW)

        // --- Depth/Stencil State ---
        bool depth_test_enable = true;
        bool depth_write_enable = true;
        // DepthCompareOp (Less, Equal, LessOrEqual, etc.)

        // --- Blend State ---
        // (Could be a vector for multiple render targets)
        // BlendStateDesc blend_state;

        std::string debug_name;
    };

    /**
     * @brief Initial configuration for the entire renderer.
     */
    struct RendererConfig {
        IWindow *window = nullptr;
        bool vsync = true;
        // Add other global settings here
        int width = 800; // Default width of the rendering window
        int height = 600; // Default height of the rendering window
    };
}
