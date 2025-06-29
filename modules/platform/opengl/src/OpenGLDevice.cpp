#include "OpenGLDevice.h"
#include "OpenGLDebug.h"
#include "../../../log/include/Log.h"

namespace RDE {

    // --- Helper function to translate our enums to GL enums ---
    GLenum ToGLBufferUsage(BufferUsage usage) {
        // This is a simplification. A real implementation might use different hints
        // like GL_DYNAMIC_DRAW, GL_STREAM_DRAW, etc. based on more detailed usage flags.
        return GL_STATIC_DRAW;
    }

    GLenum ToGLShaderType(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return GL_VERTEX_SHADER;
            case ShaderStage::Fragment:
                return GL_FRAGMENT_SHADER;
            case ShaderStage::Geometry:
                return GL_GEOMETRY_SHADER;
            case ShaderStage::Compute:
                return GL_COMPUTE_SHADER;
                // Add Tessellation later
            default:
                return GL_NONE;
        }
    }

    // --- Destructor ---
    OpenGLDevice::~OpenGLDevice() {
        // In a real engine, you'd loop and call glDelete* on all remaining resources.
        // For now, we rely on the destroy_* calls being made.
        RDE_CORE_INFO("OpenGLDevice destroyed");
    }

    // --- Handle Generation ---
    template<GpuHandleType T>
    GpuHandle<T> OpenGLDevice::next_handle() {
        return GpuHandle<T>{m_next_handle_id++};
    }

    // --- Buffer Implementation ---
    GpuBufferHandle OpenGLDevice::create_buffer(const BufferDesc &desc) {
        auto handle = next_handle<GpuHandleType::Buffer>();

        OpenGL_Buffer buffer;
        buffer.desc = desc;

        glCreateBuffers(1, &buffer.id);
        GL_CHECK_ERROR();
        glNamedBufferData(buffer.id, desc.size, desc.initial_data, ToGLBufferUsage(desc.usage));
        GL_CHECK_ERROR();

        m_buffers[handle.id] = buffer;
        return handle;
    }

    void OpenGLDevice::destroy_buffer(GpuBufferHandle handle) {
        if (m_buffers.count(handle.id)) {
            glDeleteBuffers(1, &m_buffers.at(handle.id).id);
            GL_CHECK_ERROR();
            m_buffers.erase(handle.id);
        }
    }

    // --- Program Implementation ---
    GpuProgramHandle OpenGLDevice::create_program(const ProgramDesc &desc) {
        auto handle = next_handle<GpuHandleType::Program>();

        OpenGL_Program program;
        program.desc = desc;
        program.id = glCreateProgram();
        GL_CHECK_ERROR();

        std::vector<GLuint> shader_ids;
        for (const auto &module_desc: desc.modules) {
            GLuint shader = glCreateShader(ToGLShaderType(module_desc.stage));
            GL_CHECK_ERROR();
            const char *source_ptr = module_desc.source.data();
            glShaderSource(shader, 1, &source_ptr, NULL);
            GL_CHECK_ERROR();
            glCompileShader(shader);
            GL_CHECK_ERROR();

            // Error checking
            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            GL_CHECK_ERROR();
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                GL_CHECK_ERROR();
                RDE_CORE_ERROR("OpenGLDevice: Shader compilation failed for {}: {}", desc.debug_name, infoLog);

                glDeleteShader(shader);
                GL_CHECK_ERROR();
                // Cleanup other shaders and the program
                for (GLuint id: shader_ids) {
                    glDeleteShader(id);
                    GL_CHECK_ERROR();
                }
                glDeleteProgram(program.id);
                GL_CHECK_ERROR();
                return {}; // Return invalid handle
            }
            glAttachShader(program.id, shader);
            GL_CHECK_ERROR();
            shader_ids.push_back(shader);
        }

        glLinkProgram(program.id);
        GL_CHECK_ERROR();

        // Error checking
        GLint success;
        glGetProgramiv(program.id, GL_LINK_STATUS, &success);
        GL_CHECK_ERROR();
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program.id, 512, NULL, infoLog);
            GL_CHECK_ERROR();
            RDE_CORE_ERROR("OpenGLDevice: Program linking failed for {}: {}", desc.debug_name, infoLog);
        }

        // Shaders are now linked into the program, we can delete them.
        for (GLuint id: shader_ids) {
            glDetachShader(program.id, id);
            GL_CHECK_ERROR();
            glDeleteShader(id);
            GL_CHECK_ERROR();
        }

        m_programs[handle.id] = program;
        return handle;
    }

    void OpenGLDevice::destroy_program(GpuProgramHandle handle) {
        if (m_programs.count(handle.id)) {
            glDeleteProgram(m_programs.at(handle.id).id);
            GL_CHECK_ERROR();
            m_programs.erase(handle.id);
        }
    }

    // --- Geometry (VAO) Implementation ---
    GpuGeometryHandle OpenGLDevice::create_geometry(const GeometryDesc &desc) {
        auto handle = next_handle<GpuHandleType::Geometry>();

        GLuint vao_id;
        glCreateVertexArrays(1, &vao_id);
        GL_CHECK_ERROR();

        const auto &vertex_buffer_gl = get_buffer(desc.vertex_buffer);
        glVertexArrayVertexBuffer(vao_id, 0, vertex_buffer_gl.id, 0, 0 /* Stride comes from pipeline */);
        GL_CHECK_ERROR();
        if (desc.index_buffer.is_valid()) {
            const auto &index_buffer_gl = get_buffer(desc.index_buffer);
            glVertexArrayElementBuffer(vao_id, index_buffer_gl.id);
            GL_CHECK_ERROR();
        }

        m_geometries[handle.id] = vao_id;
        return handle;
    }

    void OpenGLDevice::destroy_geometry(GpuGeometryHandle handle) {
        if (m_geometries.count(handle.id)) {
            glDeleteVertexArrays(1, &m_geometries.at(handle.id));
            GL_CHECK_ERROR();
            m_geometries.erase(handle.id);
        }
    }

    // --- Pipeline Implementation ---
    GpuPipelineHandle OpenGLDevice::create_graphics_pipeline(const GraphicsPipelineDesc &desc) {
        // For OpenGL, this is a "logical" object. We just store the description.
        // The command buffer will apply these states when bind_pipeline is called.
        auto handle = next_handle<GpuHandleType::Pipeline>();
        m_pipelines[handle.id] = {desc};
        return handle;
    }

    void OpenGLDevice::destroy_graphics_pipeline(GpuPipelineHandle handle) {
        m_pipelines.erase(handle.id);
    }

    // --- Texture Implementation (Simplified) ---
    GpuTextureHandle OpenGLDevice::create_texture(const TextureDesc &desc) {
        // Implementation for create_texture, destroy_texture would go here
        // using glCreateTextures, glTextureStorage2D, glTextureSubImage2D etc.
        return {}; // Placeholder
    }

    void OpenGLDevice::destroy_texture(GpuTextureHandle handle) {
        // glDeleteTextures
    }

    // --- Getters for Command Buffer ---
    const OpenGL_Texture &OpenGLDevice::get_texture(GpuTextureHandle handle) const { return m_textures.at(handle.id); }

    const OpenGL_Buffer &OpenGLDevice::get_buffer(GpuBufferHandle handle) const { return m_buffers.at(handle.id); }

    const OpenGL_Program &OpenGLDevice::get_program(GpuProgramHandle handle) const { return m_programs.at(handle.id); }

    const OpenGL_Pipeline &OpenGLDevice::get_pipeline(GpuPipelineHandle handle) const {
        return m_pipelines.at(handle.id);
    }

}