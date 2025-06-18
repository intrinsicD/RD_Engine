// RDE_Project/modules/platform/opengl/OpenGLShader.h
#pragma once
#include "Renderer/Shader.h"
#include <glad/gl.h> // OpenGL header

class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);

    ~OpenGLShader() override;

    void Bind() const override;

    void Unbind() const override;

    void SetMat4(const std::string &name, const glm::mat4 &matrix) override;

    void SetIntArray(const std::string &name, int *values, uint32_t count) override;

private:
    GLuint m_renderer_id;
};
