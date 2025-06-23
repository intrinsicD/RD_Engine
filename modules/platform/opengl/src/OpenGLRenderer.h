// RDE_Project/modules/platform/opengl/src/OpenGLRenderer.h
#pragma once

#include "IRenderer.h"
#include <glad/gl.h> // Or your preferred OpenGL loader
#include <unordered_map>

namespace RDE {
    // This is the concrete implementation for OpenGL.
    class OpenGLRenderer final : public IRenderer {
    public:
        explicit OpenGLRenderer(const RendererConfig &config);

        ~OpenGLRenderer() override;

        // --- IRenderer Interface Implementation ---
        bool init(const RendererConfig &config) override;

        void shutdown() override;

        bool begin_frame() override;

        void draw_frame(const CameraData &camera_data) override;

        void end_frame() override;

        void submit(const RenderObject &render_object) override;

        void submit_batch(const std::vector<RenderObject> &render_objects) override;

        void submit_instanced(const InstancedRenderObject &instanced_object) override;

        void submit_indirect(const IndirectRenderObject &indirect_command) override;

        GeometryHandle create_geometry(const GeometryData &geometry_data) override;

        TextureHandle create_texture(const TextureData &texture_data) override;

        MaterialHandle create_material(const MaterialData &material_data) override;

        ProgramHandle create_program(const ShaderData &shader_data) override;

        BufferHandle create_buffer(const BufferData &buffer_data) override;

        void destroy_geometry(GeometryHandle handle) override;

        void destroy_texture(TextureHandle handle) override;

        void destroy_material(MaterialHandle handle) override;

        void destroy_program(ProgramHandle handle) override;

        void destroy_buffer(BufferHandle handle) override;

        void on_window_resize(uint32_t width, uint32_t height) override;

    private:
        // --- Internal OpenGL-specific data structures ---
        // A VAO is essential in modern OpenGL. It bundles all the state needed to draw a mesh:
        // which VBO, which EBO, and how the vertex attributes are laid out.
        struct GLGeometry {
            GLuint vao = 0; // Vertex Array Object
            GLuint vbo = 0; // Vertex Buffer Object
            GLuint ebo = 0; // Element Buffer Object
            GLsizei index_count = 0;
        };

        struct GLBuffer {
            GLuint id = 0;
            GLenum type; // Type of buffer (Vertex, Index, etc.)
        };

        struct GLTexture {
            GLuint id = 0;
            unsigned int width = 0;
            unsigned int height = 0;
        };

        // In OpenGL, a Material is just a CPU-side description of the state
        // that needs to be set before drawing.
        using GLMaterial = MaterialData;

        // --- Private Helper Functions ---
        GLuint compile_shader(const std::string &source, GLenum type);

        GLenum get_gl_topology(PrimitiveTopologyType topology) const;

        GLenum get_gl_buffer_type(BufferType type) const;

        // --- Member Variables ---
        RendererConfig m_config;
        uint64_t m_next_handle_id = 1; // For generating unique public handles

        // The "pools" mapping our public handles to internal OpenGL object IDs.
        std::unordered_map<GeometryHandle, GLGeometry> m_geometries;
        std::unordered_map<TextureHandle, GLuint> m_textures;
        std::unordered_map<MaterialHandle, GLMaterial> m_materials;
        std::unordered_map<ProgramHandle, GLuint> m_programs;
        std::unordered_map<BufferHandle, GLBuffer> m_buffers;

        // The queue of objects to be rendered this frame.
        std::vector<RenderObject> m_render_queue;
        std::vector<InstancedRenderObject> m_instanced_render_queue;
        std::vector<IndirectRenderObject> m_indirect_render_queue;
    };
}
