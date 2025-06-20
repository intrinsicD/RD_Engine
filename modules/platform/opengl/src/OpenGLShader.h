// RDE_Project/modules/platform/opengl/OpenGLShader.h
#pragma once
#include "Shader.h"
#include <glad/gl.h> // OpenGL header

namespace RDE {
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);

        ~OpenGLShader() override;

        void bind() const override;

        void unbind() const override;

        void set_mat4(const std::string &name, const glm::mat4 &matrix) override;

        void set_int(const std::string &name, int value) override;

        void set_int_array(const std::string &name, int *values, uint32_t count) override;

        void set_float(const std::string &name, const glm::vec3 &data) override;

        void set_float(const std::string &name, const glm::vec4 &data) override;

    private:
        GLuint m_renderer_id;
    };
}
