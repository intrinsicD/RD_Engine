#include "OpenGLRenderer.h"
#include "Log.h"

#include "ShaderAsset.h"
#include "MeshAsset.h"
#include "MaterialAsset.h"
#include "TextureAsset.h"

#include "OpenGLDebug.h"

namespace RDE {
    OpenGLRenderer::OpenGLRenderer() {
    }

    bool CheckCompileErrors(GLuint shader_id, GLenum shader_type) {
        GLint isCompiled = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &isCompiled);
        GL_CHECK_ERROR();
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);
            GL_CHECK_ERROR();
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(shader_id, maxLength, &maxLength, &infoLog[0]);
            GL_CHECK_ERROR();
            glDeleteShader(shader_id);
            GL_CHECK_ERROR();
            std::string shaderTypeString = GLGetShaderTypeString(shader_type);
            RDE_CORE_ERROR("{} compilation failure: {}", shaderTypeString, infoLog.data());
            RDE_CORE_ASSERT(false, "{} compilation failure!", shaderTypeString);
            return false;
        }
        return true;
    }

    bool CheckLinkErrors(GLuint program_id) {
        GLint isLinked = 0;
        glGetProgramiv(program_id, GL_LINK_STATUS, &isLinked);
        GL_CHECK_ERROR();
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);
            GL_CHECK_ERROR();
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]);
            GL_CHECK_ERROR();
            glDeleteProgram(program_id);
            GL_CHECK_ERROR();
            RDE_CORE_ERROR("Shader link failure: {}", infoLog.data());
            RDE_CORE_ASSERT(false, "Shader link failure!");
            return false;
        }
        return true;
    }

    GLuint CreateAndCompileShader(GLenum shader_type, const std::string &source) {
        GLuint shader_id = glCreateShader(shader_type);
        GL_CHECK_ERROR();

        const char *shader_source = source.c_str();
        glShaderSource(shader_id, 1, &shader_source, NULL);
        GL_CHECK_ERROR();

        glCompileShader(shader_id);
        GL_CHECK_ERROR();
        if (!CheckCompileErrors(shader_id, shader_type)) {
            return 0; // Compilation failed
        }
        return shader_id;
    }

    bool OpenGLRenderer::compile_shader(ShaderAsset *shader_asset) {
        if (!shader_asset) {
            RDE_CORE_ERROR("OpenGL Renderer::compile_shader: null shader");
            return false;
        }

        if (shader_asset->renderer_id != 0) {
            RDE_CORE_WARN("OpenGL Renderer::compile_shader: Shader already compiled, skipping.");
            return true; // Already compiled
        }

        if (!shader_asset->compute_source.empty()) {
            GLuint shader_id = CreateAndCompileShader(GL_COMPUTE_SHADER, shader_asset->compute_source);
            if (shader_id == 0) {
                return false;
            }

            GLuint program_id = glCreateProgram();
            GL_CHECK_ERROR();
            glAttachShader(program_id, shader_id);
            GL_CHECK_ERROR();
            glLinkProgram(program_id);
            GL_CHECK_ERROR();

            if (CheckLinkErrors(program_id)) {
                return false;
            }

            glDetachShader(program_id, shader_id);
            GL_CHECK_ERROR();
            shader_asset->renderer_id = program_id;
            RDE_CORE_INFO("Successfully created OpenGL Compute Shader (ID: {0})", program_id);
            return true;
        }

        //now we handle the vertex and fragment shaders
        GLuint vs_id = 0;
        if (!shader_asset->vertex_source.empty()) {
            vs_id = CreateAndCompileShader(GL_VERTEX_SHADER, shader_asset->vertex_source);
            if (vs_id == 0) {
                return false;
            }
        }
        GLuint fs_id = 0;
        if (!shader_asset->fragment_source.empty()) {
            fs_id = CreateAndCompileShader(GL_FRAGMENT_SHADER, shader_asset->fragment_source);
            if (fs_id == 0) {
                return false;
            }
        }
        GLuint gs_id = 0;
        if (!shader_asset->geometry_source.empty()) {
            gs_id = CreateAndCompileShader(GL_GEOMETRY_SHADER, shader_asset->geometry_source);
            if (gs_id == 0) {
                return false;
            }
        }

        GLuint program_id = glCreateProgram();
        GL_CHECK_ERROR();
        if (vs_id != 0) {
            glAttachShader(program_id, vs_id);
            GL_CHECK_ERROR();
        }
        if (fs_id != 0) {
            glAttachShader(program_id, fs_id);
            GL_CHECK_ERROR();
        }
        if (gs_id != 0) {
            glAttachShader(program_id, gs_id);
            GL_CHECK_ERROR();
        }

        glLinkProgram(program_id);
        if (!CheckLinkErrors(program_id)) {
            return false; // Linking failed
        }
        if (vs_id != 0) {
            glDetachShader(program_id, vs_id);
            GL_CHECK_ERROR();
        }
        if (fs_id != 0) {
            glDetachShader(program_id, fs_id);
            GL_CHECK_ERROR();
        }
        if (gs_id != 0) {
            glDetachShader(program_id, gs_id);
            GL_CHECK_ERROR();
        }

        shader_asset->renderer_id = program_id;
        RDE_CORE_INFO("Successfully created OpenGL Shader (ID: {0})", program_id);
        return true; // Compilation and linking succeeded
    }

    void OpenGLRenderer::bind_material(const MaterialAsset *material, AssetManager &asset_manager) {
        // 1. Get the shader asset from the material's handle
        ShaderAsset *shader = asset_manager.get<ShaderAsset>(material->shader_handle);
        if (!shader || shader->renderer_id == 0) {
            // Handle error: shader not loaded or not compiled
            return;
        }

        // 2. Bind the shader program ("bind" action)
        glUseProgram(shader->renderer_id);

        // 3. Set all uniforms from the material's parameters ("set_uniform" actions)
        int texture_unit = 0;
        for (const auto &[name, param]: material->parameters) {
            GLint location = glGetUniformLocation(shader->renderer_id, name.c_str());
            if (location == -1) continue; // Skip if uniform not found

            std::visit([&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, float>)
                    glUniform1f(location, arg);
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    glUniform3fv(location, 1, &arg[0]);
                else if constexpr (std::is_same_v<T, glm::vec4>)
                    glUniform4fv(location, 1, &arg[0]);
                else if constexpr (std::is_same_v<T, AssetHandle>) {
                    // This is a texture handle
                    TextureAsset *texture = asset_manager.get<TextureAsset>(arg);
                    if (texture /* && texture->renderer_id != 0 */) {
                        glActiveTexture(GL_TEXTURE0 + texture_unit);
                        glBindTexture(GL_TEXTURE_2D, texture->renderer_id);
                        glUniform1i(location, texture_unit);
                        texture_unit++;
                    }
                }
            }, param);
        }
    }

    bool OpenGLRenderer::upload_mesh(MeshAsset *mesh_asset) {
        if (!mesh_asset || mesh_asset->vao_id != 0) {
            // Do not re-upload if already on GPU.
            return mesh_asset != nullptr;
        }

        // 1. Create VAO, VBO, EBO
        glGenVertexArrays(1, &mesh_asset->vao_id);
        glGenBuffers(1, &mesh_asset->vbo_id);
        glGenBuffers(1, &mesh_asset->ebo_id);

        // 2. Bind VAO and upload data
        glBindVertexArray(mesh_asset->vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, mesh_asset->vbo_id);
        glBufferData(GL_ARRAY_BUFFER, mesh_asset->vertices.size() * sizeof(Vertex), mesh_asset->vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_asset->ebo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_asset->indices.size() * sizeof(uint32_t), mesh_asset->indices.data(),
                     GL_STATIC_DRAW);

        // 3. Set vertex attribute pointers
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        // Tex Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tex_coords));

        // 4. Unbind VAO for safety
        glBindVertexArray(0);

        RDE_CORE_INFO("MeshAsset uploaded to GPU (VAO ID: {})", mesh_asset->vao_id);
        return true;
    }

    bool OpenGLRenderer::upload_texture(TextureAsset *texture_asset) {
        if (!texture_asset || texture_asset->renderer_id != 0) {
            return texture_asset != nullptr;
        }

        GLenum internal_format = 0, data_format = 0;
        if (texture_asset->channels == 4) {
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        } else if (texture_asset->channels == 3) {
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        } else {
            RDE_CORE_ERROR("Unsupported texture channel count: {}", texture_asset->channels);
            return false;
        }

        glGenTextures(1, &texture_asset->renderer_id);
        glBindTexture(GL_TEXTURE_2D, texture_asset->renderer_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture_asset->width, texture_asset->height, 0, data_format,
                     GL_UNSIGNED_BYTE, texture_asset->pixel_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        RDE_CORE_INFO("TextureAsset uploaded to GPU (ID: {})", texture_asset->renderer_id);
        return true;
    }

    bool OpenGLRenderer::upload_camera(const CameraComponent *camera) {
        if (m_camera_ubo == 0) {
            // Create the UBO for camera data
            glGenBuffers(1, &m_camera_ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraComponent), nullptr, GL_DYNAMIC_DRAW); // Allocate memory
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraComponent), camera);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_camera_ubo); // Bind to binding point 0
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // --- Scene Drawing Interface ---
    void OpenGLRenderer::begin_scene(const TransformComponent &camera_transform,
                                     const CameraProjectionComponent &camera_projection,
                                     const CameraComponent &camera) {
        //update ubo for camera.
        upload_camera(&camera);
    }

    void OpenGLRenderer::submit(const std::vector<DrawCommand> &commands) {
        auto &asset_manager =
        for (const auto& command : commands) {
            // The material asset should be valid here, checked by RenderSystem
            ShaderAsset* shader = m_asset_manager->Get<ShaderAsset>(command.material->shader_handle);
            if (!shader || shader->renderer_id == 0) continue;

            // Bind the material, which sets the shader and texture/color uniforms
            bind_material(command.material, *m_asset_manager);

            // Set the per-object model matrix uniform
            GLint model_loc = glGetUniformLocation(shader->renderer_id, "u_model");
            if (model_loc != -1) {
                glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(command.transform));
            }

            // Bind the mesh's VAO
            // The mesh asset must be valid and uploaded here
            glBindVertexArray(command.mesh->vao_id);

            // Issue the draw call!
            glDrawElements(GL_TRIANGLES, command.mesh->indices.size(), GL_UNSIGNED_INT, 0);
        }

        // Unbind for safety after the loop
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void OpenGLRenderer::end_scene() {
    }
}
