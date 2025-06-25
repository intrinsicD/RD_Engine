#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace RDE {
    enum class HandleType : uint8_t { Geometry, Texture, Material, Program, Buffer };

    enum class ShaderType : uint8_t {
        Vertex, Fragment, Geometry, Compute, TessellationControl, TessellationEvaluation
    };

    enum class BufferType : uint8_t { Uniform, Storage, Indirect };

    enum class PrimitiveTopologyType : uint8_t { Points, Lines, LineStrip, Triangles, TriangleStrip, TriangleFan };

    template<HandleType T>
    class Handle {
    public:
        uint64_t id = 0;
        bool is_valid() const { return id != 0; }
        // Add comparison operators for use in maps, etc.
        bool operator==(const Handle &other) const {
            return id == other.id;
        }

        bool operator!=(const Handle &other) const {
            return id != other.id;
        }
    };

    using GpuGeometryHandle = Handle<HandleType::Geometry>;
    using GpuTextureHandle = Handle<HandleType::Texture>;
    using GpuMaterialHandle = Handle<HandleType::Material>;
    using GpuProgramHandle = Handle<HandleType::Program>;
    using GpuBufferHandle = Handle<HandleType::Buffer>;

    struct RendererConfig {
        void *window_handle = nullptr; // Handle to the window where rendering will occur
        int width = 800; // Default width of the rendering window
        int height = 600; // Default height of the rendering window
        bool vsync = true; // Vertical sync setting to prevent screen tearing
        // Additional configuration options can be added here, such as:
        // - Renderer API (OpenGL, Vulkan, etc.)

        //How do i do exactly that?

        enum class Api {
            OpenGL,
            Vulkan,
        } api;

        //additional version string?
        std::string version = "4.5"; // Version of the renderer, useful for debugging or logging
    };

    struct CameraData {
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal; // Normal vector for lighting calculations
        glm::vec2 tex_coords; // Texture coordinates for UV mapping
    };

    struct GeometryData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices; // Indices for indexed drawing
    };

    struct TextureData {
        int width = 0; // Width of the texture
        int height = 0; // Height of the texture
        int channels = 0; // Number of color channels (e.g., 3 for RGB, 4 for RGBA)
        bool generate_mipmaps = true; // Whether to generate mipmaps for the texture
        // Additional properties like filtering, wrapping modes, etc. can be added here
        std::vector<uint8_t> data; // Raw data for the texture
    };

    struct MaterialData {
        PrimitiveTopologyType topology = PrimitiveTopologyType::Triangles;

        // A material can have multiple shader programs for different stages.
        std::unordered_map<ShaderType, GpuProgramHandle> programs;

        // A material can have textures...
        std::unordered_map<std::string, GpuTextureHandle> textures;
        // ...and it can also have generic storage and uniform buffers!
        std::unordered_map<std::string, GpuBufferHandle> storage_buffers;
        std::unordered_map<std::string, GpuBufferHandle> uniform_buffers;
    };

    struct ShaderData {
        std::unordered_map<ShaderType, std::string> sources;
        std::vector<std::pair<std::string, std::string> > defines; // Preprocessor defines for shader compilation
        // Additional properties like shader version, entry points, etc. can be added here
    };

    struct BufferData {
        std::vector<uint8_t> data; // Raw data for the buffer
        size_t size = 0; // Size of the buffer in bytes
        bool dynamic = false; // Whether the buffer is dynamic (can change frequently)
        bool persistent = false; // Whether the buffer should persist across frames
        // Additional properties like usage flags, memory hints, etc. can be added here
        BufferType type = BufferType::Storage; // Type of the buffer (e.g., storage, uniform or indirect)
    };

    struct RenderObject {
        GpuGeometryHandle geometry;
        GpuMaterialHandle material;
        glm::mat4 model_matrix; // Model matrix for transforming the object in world space
        // Additional properties like texture handles, custom data, etc. can be added here
    };

    struct InstancedRenderObject {
        GpuGeometryHandle geometry;
        GpuMaterialHandle material;

        GpuBufferHandle transform_buffer;
        uint32_t instance_count = 1;
    };

    struct IndirectRenderObject {
        GpuMaterialHandle material; // The material (and thus PSO) to use for all draws.

        // A buffer containing an array of draw arguments. The GPU will read from this.
        // Each element in the buffer might be { vertex_count, instance_count, first_vertex, first_instance }.
        GpuBufferHandle arguments_buffer;

        uint32_t draw_count; // The number of draw commands in the arguments_buffer.
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // --- 1. LIFECYCLE & CONFIGURATION ---
        virtual bool init(const RendererConfig &config) = 0;

        virtual void shutdown() = 0;


        // --- 2. FRAME MANAGEMENT ---
        // Begins a new frame. Returns false if the swapchain needs to be recreated (e.g., minimized window)
        virtual bool begin_frame() = 0;

        // Executes all the rendering work for the frame.
        // This is where the magic happens internally (render graph execution).
        virtual void draw_frame(const CameraData &camera_data) = 0;

        virtual void execute_and_present() = 0;

        // Submits the final frame to the window for presentation.
        virtual void end_frame() = 0;


        // --- 3. DATA SUBMISSION ---
        // Submits a single renderable object for this frame.
        virtual void submit(const RenderObject &render_object) = 0;

        // Submits a list of objects at once (more efficient).
        virtual void submit_batch(const std::vector<RenderObject> &render_objects) = 0;

        virtual void submit_instanced(const InstancedRenderObject &instanced_object) = 0;

        virtual void submit_indirect(const IndirectRenderObject &indirect_command) = 0;


        // --- 4. RESOURCE FACTORY ---
        // The renderer is the ONLY one who can create GPU resources.
        virtual GpuGeometryHandle create_geometry(const GeometryData &geometry_data) = 0;

        virtual GpuTextureHandle create_texture(const TextureData &texture_data) = 0;

        virtual GpuMaterialHandle create_material(const MaterialData &material_data) = 0;

        virtual GpuProgramHandle create_program(const ShaderData &shader_data) = 0;

        virtual GpuBufferHandle create_buffer(const BufferData &buffer_data) = 0;

        virtual void destroy_geometry(GpuGeometryHandle handle) = 0;

        virtual void destroy_texture(GpuTextureHandle handle) = 0;

        virtual void destroy_material(GpuMaterialHandle handle) = 0;

        virtual void destroy_program(GpuProgramHandle handle) = 0;

        virtual void destroy_buffer(GpuBufferHandle handle) = 0;

        // --- 5. EVENT HANDLING ---
        // Called when the window resizes, as this requires recreating the swapchain.
        virtual void on_window_resize(uint32_t width, uint32_t height) = 0;

        // DEVELOPMENT-ONLY: Called to trigger a hot reload of shaders
        virtual void on_hot_reload_shaders() {
        }

        static std::unique_ptr<IRenderer> Create(const RendererConfig &config = RendererConfig());
    };
}

namespace std {
    // --- FIX 2: ADD A HASH SPECIALIZATION FOR THE Handle<T> TEMPLATE ---
    // This tells the standard library how to generate a hash value from your handle.
    template<RDE::HandleType T>
    struct hash<RDE::Handle<T> > {
        size_t operator()(const RDE::Handle<T> &handle) const {
            // The hash of the handle is just the hash of its underlying ID.
            // This is a simple and effective hashing strategy.
            return hash<uint64_t>()(handle.id);
        }
    };
} // namespace std
