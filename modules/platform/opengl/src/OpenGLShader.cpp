// RDE_Project/modules/platform/opengl/OpenGLShader.cpp
#include "OpenGLShader.h"
#include "OpenGLDebug.h"
#include "Log.h"
#include "FileIO.h"

namespace RDE {
// Factory function implementation is here
    std::shared_ptr<Shader> Shader::Create(const std::string &vertexSrc, const std::string &fragmentSrc) {
        return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
    }

    std::shared_ptr<Shader>
    Shader::CreateFromFile(const std::string &vertexFilepath, const std::string &fragmentFilepath) {
        std::string vertexSrc = FileIO::read_file(vertexFilepath);
        std::string fragmentSrc = FileIO::read_file(fragmentFilepath);
        RDE_CORE_ASSERT(!vertexSrc.empty(), "Vertex shader file is empty or could not be read!");
        RDE_CORE_ASSERT(!fragmentSrc.empty(), "Fragment shader file is empty or could not be read!");
        return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
    }

    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc) {
        // Create an empty program object
        GLuint program = glCreateProgram();
        GL_CHECK_ERROR();

        // Create and compile the vertex shader
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GL_CHECK_ERROR();

        const char *vs_source = vertexSrc.c_str();
        glShaderSource(vs, 1, &vs_source, NULL);
        GL_CHECK_ERROR();

        glCompileShader(vs);
        GL_CHECK_ERROR();

        GLint isCompiled = 0;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
        GL_CHECK_ERROR();
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
            GL_CHECK_ERROR();
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vs, maxLength, &maxLength, &infoLog[0]);
            GL_CHECK_ERROR();
            glDeleteShader(vs);
            GL_CHECK_ERROR();
            RDE_CORE_ERROR("Vertex shader compilation failure: {0}", infoLog.data());
            RDE_CORE_ASSERT(false, "Vertex shader compilation failure!");
            return;
        }

        // Create and compile the fragment shader
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CHECK_ERROR();
        const char *fs_source = fragmentSrc.c_str();
        glShaderSource(fs, 1, &fs_source, NULL);
        GL_CHECK_ERROR();
        glCompileShader(fs);
        GL_CHECK_ERROR();

        glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);
        GL_CHECK_ERROR();
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);
            GL_CHECK_ERROR();
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fs, maxLength, &maxLength, &infoLog[0]);
            GL_CHECK_ERROR();
            glDeleteShader(fs);
            GL_CHECK_ERROR();
            glDeleteShader(vs);
            GL_CHECK_ERROR();
            RDE_CORE_ERROR("Fragment shader compilation failure: {0}", infoLog.data());
            RDE_CORE_ASSERT(false, "Fragment shader compilation failure!");
            return;
        }

        // Link the shaders into a program
        glAttachShader(program, vs);
        GL_CHECK_ERROR();
        glAttachShader(program, fs);
        GL_CHECK_ERROR();
        glLinkProgram(program);
        GL_CHECK_ERROR();
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int *) &isLinked);
        GL_CHECK_ERROR();
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
            GL_CHECK_ERROR();
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
            GL_CHECK_ERROR();
            glDeleteProgram(program);
            GL_CHECK_ERROR();
            glDeleteShader(vs);
            GL_CHECK_ERROR();
            glDeleteShader(fs);
            GL_CHECK_ERROR();
            RDE_CORE_ERROR("Shader link failure: {0}", infoLog.data());
            RDE_CORE_ASSERT(false, "Shader link failure!");
            return;
        }

        // Detach shaders after a successful link
        glDetachShader(program, vs);
        GL_CHECK_ERROR();
        glDetachShader(program, fs);
        GL_CHECK_ERROR();
        m_renderer_id = program;
        RDE_CORE_INFO("Successfully created OpenGL Shader (ID: {0})", m_renderer_id);
    }

    OpenGLShader::~OpenGLShader() {
        if (m_renderer_id) {
            glDeleteProgram(m_renderer_id);
            GL_CHECK_ERROR();
        }
    }

    void OpenGLShader::bind() const {
        glUseProgram(m_renderer_id);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::unbind() const {
        glUseProgram(0);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::set_mat4(const std::string &name, const glm::mat4 &matrix) {
        GLint location = glGetUniformLocation(m_renderer_id, name.c_str());
        GL_CHECK_ERROR();

        glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::set_int(const std::string &name, int value) {
        GLint location = glGetUniformLocation(m_renderer_id, name.c_str());
        GL_CHECK_ERROR();
        glUniform1i(location, value);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::set_int_array(const std::string &name, int *values, uint32_t count) {
        GLint location = glGetUniformLocation(m_renderer_id, name.c_str());
        GL_CHECK_ERROR();
        glUniform1iv(location, count, values);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::set_float(const std::string &name, const glm::vec3 &data) {
        GLint location = glGetUniformLocation(m_renderer_id, name.c_str());
        GL_CHECK_ERROR();
        glUniform3fv(location, 1, &data[0]);
        GL_CHECK_ERROR();
    }

    void OpenGLShader::set_float(const std::string &name, const glm::vec4 &data) {
        GLint location = glGetUniformLocation(m_renderer_id, name.c_str());
        GL_CHECK_ERROR();
        glUniform4fv(location, 1, &data[0]);
        GL_CHECK_ERROR();
    }
}