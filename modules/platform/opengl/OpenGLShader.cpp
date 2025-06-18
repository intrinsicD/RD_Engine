// RDE_Project/modules/platform/opengl/OpenGLShader.cpp
#include "OpenGLShader.h"
#include "Core/Log.h"

// Factory function implementation is here
std::unique_ptr<Shader> Shader::Create(const std::string &vertexSrc, const std::string &fragmentSrc) {
    return std::make_unique<OpenGLShader>(vertexSrc, fragmentSrc);
}

OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc) {
    // ... (Full OpenGL shader compilation logic goes here)
    // 1. Create vertex and fragment shaders (glCreateShader)
    // 2. Set source (glShaderSource) and compile (glCompileShader)
    // 3. Check for compile errors (glGetShaderiv, glGetShaderInfoLog)
    // 4. Create program, attach shaders, and link (glCreateProgram, glAttachShader, glLinkProgram)
    // 5. Check for link errors (glGetProgramiv, glGetProgramInfoLog)
    // 6. Detach and delete shaders
    // The resulting program ID is stored in m_renderer_id.

    // For brevity, a placeholder:
    RDE_CORE_INFO("Creating OpenGL Shader...");
    m_renderer_id = glCreateProgram(); // This is just a placeholder
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(m_renderer_id);
}

void OpenGLShader::Bind() const {
    glUseProgram(m_renderer_id);
}

void OpenGLShader::Unbind() const {
    glUseProgram(0);
}
