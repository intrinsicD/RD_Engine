// RDE_Project/modules/platform/src/OpenGLRenderer.cpp

#include "OpenGLRenderer.h"
#include "Log.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace RDE {
    std::unique_ptr<IRenderer> IRenderer::Create(const RendererConfig& config) {
        switch (config.api) {
            case RendererConfig::Api::OpenGL:
                // It creates and returns a unique_ptr to the OpenGL implementation.
                return std::make_unique<OpenGLRenderer>(config);

            case RendererConfig::Api::Vulkan:
                // In the future, you would just add this case:
                // return std::make_unique<VulkanRenderer>();
                RDE_CORE_ASSERT(false, "Vulkan renderer not yet implemented!");
                return nullptr;
        }
        RDE_CORE_ASSERT(false, "Unknown renderer API specified!");
        return nullptr;
    }

    OpenGLRenderer::OpenGLRenderer(const RendererConfig &config) {
        init(config);
    }

    OpenGLRenderer::~OpenGLRenderer() {
        shutdown();
    }

    bool OpenGLRenderer::init(const RendererConfig &config) {
        m_config = config;

        // Initialize GLAD

        int status = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
        if (!status) {
            // Assuming GLFW
            RDE_CORE_ASSERT(status, "Failed to initialize Glad!");
            return false;
        }

        RDE_CORE_INFO("OpenGL Renderer vendor: {}", (const char*)glGetString(GL_VENDOR));

        // Set initial GL state that rarely changes
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void OpenGLRenderer::shutdown() {
        // Clean up all GPU resources
        for (auto const &[handle, geo]: m_geometries) {
            glDeleteVertexArrays(1, &geo.vao);
            glDeleteBuffers(1, &geo.vbo);
            glDeleteBuffers(1, &geo.ebo);
        }
        for (auto const &[handle, id]: m_textures)
            glDeleteTextures(1, &id);
        for (auto const &[handle, id]: m_programs)
            glDeleteProgram(id);
        for (auto const &[handle, data]: m_materials) {
        }
        // ... etc. for other resources
    }

    bool OpenGLRenderer::begin_frame() {
        m_render_queue.clear();
        // In a more complex app, you might check for context loss here
        return true;
    }

    void OpenGLRenderer::end_frame() {

    }

    void OpenGLRenderer::submit(const RenderObject &render_object) {
        m_render_queue.push_back(render_object);
    }

    void OpenGLRenderer::submit_batch(const std::vector<RenderObject> &render_objects) {
        // This is a more efficient way to submit multiple objects at once.
        // It avoids multiple calls to submit, which can be costly.
        m_render_queue.insert(m_render_queue.end(), render_objects.begin(), render_objects.end());
    }

    void OpenGLRenderer::submit_instanced(const InstancedRenderObject &instanced_object) {
        // For instanced rendering, we would typically use a different draw call.
        // Here, we just push it to the render queue.
        m_instanced_render_queue.push_back(instanced_object);
    }

    void OpenGLRenderer::submit_indirect(const IndirectRenderObject &indirect_command) {
        // Indirect rendering is a bit more complex. We would typically use a buffer
        // to store the draw commands and then issue a single draw call.
        // Here, we just push it to the render queue.
        m_indirect_render_queue.push_back(indirect_command);
    }

    GpuGeometryHandle OpenGLRenderer::create_geometry(const GeometryData &geometry_data) {
        GLGeometry new_geo;
        new_geo.index_count = geometry_data.indices.size();

        // 1. Create VAO, VBO, and EBO
        glGenVertexArrays(1, &new_geo.vao);
        glGenBuffers(1, &new_geo.vbo);
        glGenBuffers(1, &new_geo.ebo);

        // 2. Bind the VAO to record state
        glBindVertexArray(new_geo.vao);

        // 3. Upload Vertex Data
        glBindBuffer(GL_ARRAY_BUFFER, new_geo.vbo);
        glBufferData(GL_ARRAY_BUFFER, geometry_data.vertices.size() * sizeof(Vertex), geometry_data.vertices.data(),
                     GL_STATIC_DRAW);

        // 4. Upload Index Data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_geo.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry_data.indices.size() * sizeof(uint32_t),
                     geometry_data.indices.data(), GL_STATIC_DRAW);

        // 5. Set Vertex Attribute Pointers. This state is SAVED in the VAO.
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        // Tex Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tex_coords));

        // 6. Unbind the VAO to prevent accidental modification
        glBindVertexArray(0);

        // 7. Store and return handle
        GpuGeometryHandle handle;
        handle.id = m_next_handle_id++;
        m_geometries[handle] = new_geo;
        return handle;
    }

    GpuTextureHandle OpenGLRenderer::create_texture(const TextureData &texture_data) {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload texture data
        if (!texture_data.data.empty()) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_data.width, texture_data.height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, texture_data.data.data());
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            RDE_CORE_ERROR("Texture data is null!");
            return GpuTextureHandle(); // Return an invalid handle
        }

        // Unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);

        GpuTextureHandle handle;
        handle.id = m_next_handle_id++;
        m_textures[handle] = texture_id;
        return handle;
    }

    GpuMaterialHandle OpenGLRenderer::create_material(const MaterialData &material_data) {
        // For OpenGL, creating a material is simple. We just store its description.
        // The real work happens at draw time.
        GpuMaterialHandle handle;
        handle.id = m_next_handle_id++;
        m_materials[handle] = material_data; // Store a copy of the data
        return handle;
    }

    GLuint OpenGLRenderer::compile_shader(const std::string &source, GLenum type) {
        GLuint shader_id = glCreateShader(type);
        const char *source_cstr = source.c_str();
        glShaderSource(shader_id, 1, &source_cstr, nullptr);
        glCompileShader(shader_id);

        // Check for compilation errors
        GLint success;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint log_length;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> log(log_length);
            glGetShaderInfoLog(shader_id, log_length, nullptr, log.data());
            RDE_CORE_ERROR("Shader compilation failed: {0}", log.data());
            glDeleteShader(shader_id);
            return 0; // Return 0 to indicate failure
        }

        return shader_id;
    }


    GpuProgramHandle OpenGLRenderer::create_program(const ShaderData &shader_data) {
        GLuint vert_shader = compile_shader(shader_data.sources.at(ShaderType::Vertex), GL_VERTEX_SHADER);
        GLuint frag_shader = compile_shader(shader_data.sources.at(ShaderType::Fragment), GL_FRAGMENT_SHADER);
        GLuint geom_shader = 0;
        if (shader_data.sources.find(ShaderType::Geometry) != shader_data.sources.end()) {
            geom_shader = compile_shader(shader_data.sources.at(ShaderType::Geometry), GL_GEOMETRY_SHADER);
        }
        GLuint tess_eval_shader = 0;
        if (shader_data.sources.find(ShaderType::TessellationEvaluation) != shader_data.sources.end()) {
            tess_eval_shader = compile_shader(shader_data.sources.at(ShaderType::TessellationEvaluation),
                                              GL_TESS_EVALUATION_SHADER);
        }
        GLuint tess_control_shader = 0;
        if (shader_data.sources.find(ShaderType::TessellationControl) != shader_data.sources.end()) {
            tess_control_shader = compile_shader(shader_data.sources.at(ShaderType::TessellationControl),
                                                 GL_TESS_CONTROL_SHADER);
        }
        // ... compile other shader stages if they exist ...

        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, vert_shader);
        glAttachShader(program_id, frag_shader);
        if (geom_shader) {
            glAttachShader(program_id, geom_shader);
        }
        if (tess_eval_shader) {
            glAttachShader(program_id, tess_eval_shader);
        }
        if (tess_control_shader) {
            glAttachShader(program_id, tess_control_shader);
        }
        glLinkProgram(program_id);

        // Check for linking errors...

        // Detach and delete shaders as they are now linked into the program
        glDetachShader(program_id, vert_shader);
        glDeleteShader(vert_shader);
        glDetachShader(program_id, frag_shader);
        glDeleteShader(frag_shader);
        if (geom_shader) {
            glDetachShader(program_id, geom_shader);
            glDeleteShader(geom_shader);
        }
        if (tess_eval_shader) {
            glDetachShader(program_id, tess_eval_shader);
            glDeleteShader(tess_eval_shader);
        }
        if (tess_control_shader) {
            glDetachShader(program_id, tess_control_shader);
            glDeleteShader(tess_control_shader);
        }

        GpuProgramHandle handle;
        handle.id = m_next_handle_id++;
        m_programs[handle] = program_id;
        return handle;
    }

    GpuBufferHandle OpenGLRenderer::create_buffer(const BufferData &buffer_data) {
        GLuint buffer_id;
        auto gl_buffer_type = get_gl_buffer_type(buffer_data.type);
        glGenBuffers(1, &buffer_id);
        glBindBuffer(gl_buffer_type, buffer_id);
        glBufferData(gl_buffer_type, buffer_data.data.size(), buffer_data.data.data(), GL_STATIC_DRAW);

        // Unbind the buffer
        glBindBuffer(gl_buffer_type, 0);

        GpuBufferHandle handle;
        handle.id = m_next_handle_id++;
        m_buffers[handle] = GLBuffer{buffer_id, get_gl_buffer_type(BufferType::Storage)}; // Assuming SSBO - Storage type for simplicity
        return handle;
    }


    void OpenGLRenderer::draw_frame(const CameraData &camera_data) {
        // --- 1. Clear the screen ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- 2. Process the Render Queue ---
        for (const auto &object: m_render_queue) {
            // --- 3. Get all necessary data from our internal maps ---
            const GLMaterial &material = m_materials.at(object.material);
            const GLuint program_id = m_programs.at(material.programs.at(ShaderType::Vertex)); // Simplified lookup
            const GLGeometry &geometry = m_geometries.at(object.geometry);

            // --- 4. Set OpenGL State (The State Machine) ---
            glUseProgram(program_id);

            // Set Uniforms
            glUniformMatrix4fv(glGetUniformLocation(program_id, "u_view"), 1, GL_FALSE, &camera_data.view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(program_id, "u_projection"), 1, GL_FALSE,
                               &camera_data.projection[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(program_id, "u_model"), 1, GL_FALSE, &object.model_matrix[0][0]);

            // Bind textures, buffers, etc. from the material...

            // --- 5. Bind the Geometry ---
            // This one call binds the VBO, EBO, and vertex attribute pointers!
            glBindVertexArray(geometry.vao);

            // --- 6. Issue the Draw Call ---
            // Convert our enum to GLenum (would use a helper function)
            GLenum topology = get_gl_topology(material.topology); // ... map from material.topology ...

            glDrawElements(topology, geometry.index_count, GL_UNSIGNED_INT, 0);

            // --- 7. Unbind for cleanliness ---
            glBindVertexArray(0);
        }
    }

    void OpenGLRenderer::destroy_geometry(GpuGeometryHandle handle)  {
        auto it = m_geometries.find(handle);
        if (it != m_geometries.end()) {
            const GLGeometry &geo = it->second;
            glDeleteVertexArrays(1, &geo.vao);
            glDeleteBuffers(1, &geo.vbo);
            glDeleteBuffers(1, &geo.ebo);
            m_geometries.erase(it);
        } else {
            RDE_CORE_ERROR("Attempted to destroy non-existent geometry handle: {0}", handle.id);
        }
    }

    void OpenGLRenderer::destroy_texture(GpuTextureHandle handle) {
        auto it = m_textures.find(handle);
        if (it != m_textures.end()) {
            glDeleteTextures(1, &it->second);
            m_textures.erase(it);
        } else {
            RDE_CORE_ERROR("Attempted to destroy non-existent texture handle: {0}", handle.id);
        }
    }

    void OpenGLRenderer::destroy_material(GpuMaterialHandle handle)  {
        auto it = m_materials.find(handle);
        if (it != m_materials.end()) {
            //TODO : Handle material cleanup if needed
            m_materials.erase(it);
        } else {
            RDE_CORE_ERROR("Attempted to destroy non-existent material handle: {0}", handle.id);
        }
    }

    void OpenGLRenderer::destroy_program(GpuProgramHandle handle)  {
        auto it = m_programs.find(handle);
        if (it != m_programs.end()) {
            //TODO : Handle material cleanup if needed
            m_programs.erase(it);
        } else {
            RDE_CORE_ERROR("Attempted to destroy non-existent material handle: {0}", handle.id);
        }
    }

    void OpenGLRenderer::destroy_buffer(GpuBufferHandle handle)  {
        auto it = m_buffers.find(handle);
        if (it != m_buffers.end()) {
            glDeleteBuffers(1, &it->second.id);
            m_buffers.erase(it);
        } else {
            RDE_CORE_ERROR("Attempted to destroy non-existent buffer handle: {0}", handle.id);
        }
    }

    // --- 5. EVENT HANDLING ---
    // Called when the window resizes, as this requires recreating the swapchain.
    void OpenGLRenderer::on_window_resize(uint32_t width, uint32_t height)  {
        // Update the viewport and any other necessary state
        glViewport(0, 0, width, height);
        // You might also want to update your projection matrix here
        RDE_CORE_INFO("Window resized to {0}x{1}", width, height);
    }

    GLenum OpenGLRenderer::get_gl_buffer_type(BufferType type) const  {
        switch (type) {
            case BufferType::Uniform:
                return GL_UNIFORM_BUFFER;
            case BufferType::Storage:
                return GL_SHADER_STORAGE_BUFFER;
            case BufferType::Indirect:
                return GL_DRAW_INDIRECT_BUFFER;
            default:
                RDE_CORE_ASSERT(false, "Unknown BufferType");
                return 0; // Invalid type
        }
    }

    GLenum OpenGLRenderer::get_gl_topology(PrimitiveTopologyType topology) const {
        switch (topology) {
            case PrimitiveTopologyType::Points:
                return GL_POINTS;
            case PrimitiveTopologyType::Lines:
                return GL_LINES;
            case PrimitiveTopologyType::LineStrip:
                return GL_LINE_STRIP;
            case PrimitiveTopologyType::Triangles:
                return GL_TRIANGLES;
            case PrimitiveTopologyType::TriangleStrip:
                return GL_TRIANGLE_STRIP;
            case PrimitiveTopologyType::TriangleFan:
                return GL_TRIANGLE_FAN;
            default:
                RDE_CORE_ASSERT(false, "Unknown PrimitiveTopologyType");
                return 0; // Invalid type
        }
    }


    // ... other functions like on_window_resize, destroy_*, etc. would be implemented here ...
}
