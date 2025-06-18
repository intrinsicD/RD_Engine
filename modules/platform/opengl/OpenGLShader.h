// RDE_Project/modules/platform/opengl/OpenGLShader.h
#pragma once
#include "Renderer/Shader.h"
#include <glad/gl.h> // OpenGL header

class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);

    ~OpenGLShader();

    void Bind() const override;

    void Unbind() const override;

private:
    GLuint m_renderer_id;
};
