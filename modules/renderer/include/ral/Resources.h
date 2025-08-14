// ral/Resources.h
#pragma once

#include "Common.h" // For handles
#include <vector>
#include <string>
#include <variant>

namespace RAL {
    // --- ADDED: Image Layouts for Barriers ---
    enum class ImageLayout {
        Undefined,              // Initial state, no data
        General,                // Can be used for anything (slow)
        ColorAttachment,        // For rendering to as a color buffer
        DepthStencilAttachment, // For rendering to as a depth/stencil buffer
        ShaderReadOnly,         // For sampling in a shader (texture())
        TransferSrc,            // Source of a copy operation
        TransferDst,            // Destination of a copy operation
        PresentSrc              // Ready to be presented to the screen
    };

    // --- ADDED: Access Flags for Barriers ---
    enum class AccessFlags : uint64_t {
        None = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        ColorAttachmentRead = 1 << 2,
        ColorAttachmentWrite = 1 << 3,
        DepthStencilAttachmentRead = 1 << 4,
        DepthStencilAttachmentWrite = 1 << 5,
        TransferRead = 1 << 6,
        TransferWrite = 1 << 7,
        HostRead = 1 << 8,
        HostWrite = 1 << 9,
        VertexAttributeRead = 1 << 10, // NEW
        IndexRead = 1 << 11             // NEW
    };

    ENABLE_ENUM_FLAG_OPERATORS(AccessFlags)

    // --- ADDED: Pipeline Stage Flags for Barriers ---
    enum class PipelineStageFlags : uint64_t {
        None = 0,
        TopOfPipe = 1 << 0,
        DrawIndirect = 1 << 1,
        VertexInput = 1 << 2,
        VertexShader = 1 << 3,
        FragmentShader = 1 << 4,
        EarlyFragmentTests = 1 << 5, // For depth/stencil before fragment shader
        LateFragmentTests = 1 << 6,  // For depth/stencil after fragment shader
        ColorAttachmentOutput = 1 << 7,
        ComputeShader = 1 << 8,
        Transfer = 1 << 9,
        BottomOfPipe = 1 << 10,
    };

    ENABLE_ENUM_FLAG_OPERATORS(PipelineStageFlags)

    // --- ADDED: The Barrier Description ---
    struct ResourceBarrier {
        // Defines which operations must complete...
        PipelineStageFlags srcStage;
        AccessFlags srcAccess;

        // ...before these operations can begin.
        PipelineStageFlags dstStage;
        AccessFlags dstAccess;

        // Use one of the following for resource-specific transitions
        struct TextureTransition {
            TextureHandle texture;
            ImageLayout oldLayout;
            ImageLayout newLayout;
        } textureTransition;

        // You could add BufferTransition here as well if needed, but for now
        // memory barriers (using just stages/access flags) cover most buffer cases.
    };

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
        HostVisibleCoherent, // CPU visible, for frequent updates (e.g. UBOs)
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

    enum class ImageAspect : uint32_t {
        None = 0,
        Color = 1 << 0, // Bit 0
        Depth = 1 << 1, // Bit 1
        Stencil = 1 << 2  // Bit 2
    };

    ENABLE_ENUM_FLAG_OPERATORS(ImageAspect)

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
        std::string name; // Optional semantic name for debugging
    };

    struct VertexInputBinding {
        uint32_t binding;
        uint32_t stride;
        enum class Rate { PerVertex, PerInstance } inputRate = Rate::PerVertex; // NEW
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

    struct GraphicsShaderStages {
        ShaderHandle vertexShader;
        ShaderHandle fragmentShader;
        ShaderHandle geometryShader;
        ShaderHandle tessControlShader;
        ShaderHandle tessEvalShader;
    };

    struct ComputeShaderStages {
        ShaderHandle computeShader;
    };

    struct MeshShaderStages {
        ShaderHandle taskShader;
        ShaderHandle meshShader;
    };

    struct PipelineDescription {
        std::variant<GraphicsShaderStages, ComputeShaderStages, MeshShaderStages> stages;
        std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts;
        std::vector<PushConstantRange> pushConstantRanges;
        RasterizationState rasterizationState;
        ColorBlendState colorBlendState;
        DepthStencilState depthStencilState;
        std::vector<VertexInputBinding> vertexBindings;
        std::vector<VertexInputAttribute> vertexAttributes;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        // NEW: explicit attachment formats (optional). If empty, backend may infer swapchain format.
        std::vector<Format> colorAttachmentFormats; // size = number of color attachments
        Format depthAttachmentFormat = Format::UNKNOWN; // UNKNOWN if no depth
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

    struct ImageSubresourceLayers {
        ImageAspect aspectMask = ImageAspect::Color; // e.g., RAL::ImageAspect::Color
        uint32_t mipLevel = 0; // Mip level to access
        uint32_t baseArrayLayer = 0; // First layer in the array
        uint32_t layerCount = 1; // Number of layers to access
    };

    struct BufferTextureCopy {
        // --- Source Buffer Layout ---
        uint64_t bufferOffset = 0;      // Byte offset into the source buffer where the data begins.
        uint32_t bufferRowLength = 0;   // How many pixels wide a row is in the buffer memory. 0 means tightly packed.
        uint32_t bufferImageHeight = 0; // How many rows high an image slice is in buffer memory. 0 means tightly packed.

        // --- Destination Texture Layout ---
        ImageSubresourceLayers imageSubresource; // Specifies aspect (color/depth), mip level, and array layer.
        Offset3D imageOffset = {0, 0, 0};      // The {x, y, z} offset in the destination texture.
        Extent3D imageExtent;                    // The {width, height, depth} of the region to copy.
    };
}
