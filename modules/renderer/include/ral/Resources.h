// ral/Resources.h
#pragma once

#include "Common.h" // For handles
#include <vector> // For handles
#include <string> // For handles

namespace RAL {
    // --- Resource Usage Flags ---
    enum class BufferUsage : uint32_t {
        None = 0,
        VertexBuffer = 1 << 0,
        IndexBuffer = 1 << 1,
        UniformBuffer = 1 << 2,
        StorageBuffer = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5,
    };

    ENABLE_ENUM_FLAG_OPERATORS(BufferUsage)

    enum class TextureUsage : uint32_t {
        None = 0,
        Sampled = 1 << 0,
        Storage = 1 << 1,
        ColorAttachment = 1 << 2,
        DepthStencilAttachment = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5,
    };

    ENABLE_ENUM_FLAG_OPERATORS(TextureUsage)

    enum class MemoryUsage {
        DeviceLocal, // GPU only, fastest access
        HostVisible, // CPU visible, for frequent updates (e.g. UBOs)
        // No need for separate Coherent/Cached here, the backend can decide
        // the best flags based on this high-level intent.
        CPU_To_GPU, // For buffers that will be written by CPU and read by GPU
    };

    enum class ShaderStage {
        None,
        Vertex, Fragment, Compute, // Core stages
        Geometry, TessellationControl, TessellationEvaluation, // Optional stages
        RayTracing, // Ray tracing stages
        Task, Mesh // Modern mesh pipeline stages
    };

    ENABLE_ENUM_FLAG_OPERATORS(ShaderStage)

    enum class CullMode {
        None, Front, Back, FrontAndBack
    };

    enum class PolygonMode {
        Fill, Line, Point
    };

    enum class FrontFace {
        Clockwise, CounterClockwise
    };

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
    };

    enum class BlendOp {
        Add, Subtract, ReverseSubtract, Min, Max
    };

    enum class CompareOp {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class DescriptorType {
        UniformBuffer,
        StorageBuffer,
        SampledImage, // Texture + Sampler
        StorageImage, // Texture without sampler
        Sampler, // Standalone sampler
        CombinedImageSampler, // Texture + Sampler in one binding
        // Other types like StorageBuffer, StorageImage can be added later
    };

    enum class Filter {
        Nearest, Linear
    };

    enum class SamplerAddressMode {
        Repeat, MirroredRepeat, ClampToEdge, ClampToBorder
    };

    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    struct StencilOpState {
        // We can fill these in later when we need stencil testing.
        // For now, defaults are fine.
    };

    struct DepthStencilState {
        bool depthTestEnable = false;
        bool depthWriteEnable = false;
        CompareOp depthCompareOp = CompareOp::LessOrEqual; // A sensible default for 3D rendering
        bool stencilTestEnable = false;
        StencilOpState front{};
        StencilOpState back{};
    };

    // --- Resource Description Structs ---

    struct BufferDescription {
        uint64_t size = 0;
        BufferUsage usage;
        MemoryUsage memoryUsage;
        const void *initialData = nullptr; // Add this for easier creation
    };

    struct TextureDescription {
        uint32_t width = 1, height = 1, depth = 1;
        uint32_t mipLevels = 1;
        Format format; // Use the single, unified Format enum
        TextureUsage usage;
        const void *initialData = nullptr;
        uint64_t initialDataSize = 0; // Size of initial data, if provided
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
        std::string semantic; // Optional semantic name for debugging
    };

    struct VertexInputBinding {
        uint32_t binding;
        uint32_t stride;
    };

    // Defines a range of push constants accessible to the pipeline.
    struct PushConstantRange {
        ShaderStage stages; // Which shader stages can access it
        uint32_t offset;
        uint32_t size;
        std::string name; // Optional name for debugging
    };

    struct RasterizationState {
        PolygonMode polygonMode = PolygonMode::Fill;
        CullMode cullMode = CullMode::Back;
        FrontFace frontFace = FrontFace::CounterClockwise;
        bool depthBiasEnable = false;
    };

    struct BlendAttachmentState {
        bool blendEnable = false;
        BlendFactor srcColorBlendFactor = BlendFactor::SrcAlpha;
        BlendFactor dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
        BlendOp colorBlendOp = BlendOp::Add;
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
        BlendOp alphaBlendOp = BlendOp::Add;
    };

    // We'll define a simple blend state for now. Can be expanded.
    struct ColorBlendState {
        // For simplicity, one blend state for all attachments.
        // A real engine might have a vector of these.
        BlendAttachmentState attachment;
    };

    struct PipelineDescription {
        ShaderHandle vertexShader;
        ShaderHandle fragmentShader;

        ShaderHandle geometryShader;
        ShaderHandle tessControlShader;
        ShaderHandle tessEvalShader;

        ShaderHandle computeShader;

        ShaderHandle taskShader;
        ShaderHandle meshShader;


        std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts;
        std::vector<PushConstantRange> pushConstantRanges;

        RasterizationState rasterizationState;
        ColorBlendState colorBlendState;
        DepthStencilState depthStencilState;

        std::vector<VertexInputBinding> vertexBindings;
        std::vector<VertexInputAttribute> vertexAttributes;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    };

    // --- NEW: Descriptor Set Descriptions ---

    // Describes a single binding within a descriptor set (e.g., "binding = 0")
    struct DescriptorSetLayoutBinding {
        uint32_t binding;
        DescriptorType type;
        ShaderStage stages; // Bitmask of stages that can access this binding
        std::string name;
    };

    // Describes the "shape" of a descriptor set. Pipelines are created with this.
    struct DescriptorSetLayoutDescription {
        uint32_t set;
        std::vector<DescriptorSetLayoutBinding> bindings;
    };

    // Describes a single resource to be written into an actual DescriptorSet instance.
    struct DescriptorWrite {
        uint32_t binding;
        DescriptorType type;
        // One of these should be valid depending on the type
        BufferHandle buffer;
        TextureHandle texture;
        SamplerHandle sampler;
        // You can add more specific info like buffer range/offset if needed
    };

    // Describes the contents of a specific DescriptorSet instance.
    struct DescriptorSetDescription {
        DescriptorSetLayoutHandle layout;
        std::vector<DescriptorWrite> writes;
    };

    struct SamplerDescription {
        Filter magFilter = Filter::Linear;
        Filter minFilter = Filter::Linear;
        SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
        SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
        // ... other options like anisotropy, mipmapping etc. can be added later
    };
}
