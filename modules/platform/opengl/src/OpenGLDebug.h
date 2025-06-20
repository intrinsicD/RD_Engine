// RDE_Project/modules/platform/opengl/OpenGLDebug.h

#pragma once

#include "Log.h"

#include <spdlog/fmt/fmt.h>
#include <glad/gl.h>

namespace RDE {
    // This function clears all previous OpenGL errors.
    static void GLClearError() {
        while (glGetError() != GL_NO_ERROR);
    }

    // This function checks for OpenGL errors, logs them, and returns false if an error occurred.
    static bool GLLogAnyError(const char *file, int line) {
        bool error_found = false;
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            error_found = true;
            std::string error_str;
            switch (error) {
                case GL_INVALID_ENUM:
                    error_str = "GL_INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    error_str = "GL_INVALID_VALUE";
                    break;
                case GL_INVALID_OPERATION:
                    error_str = "GL_INVALID_OPERATION";
                    break;
                case GL_STACK_OVERFLOW:
                    error_str = "GL_STACK_OVERFLOW";
                    break;
                case GL_STACK_UNDERFLOW:
                    error_str = "GL_STACK_UNDERFLOW";
                    break;
                case GL_OUT_OF_MEMORY:
                    error_str = "GL_OUT_OF_MEMORY";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error_str = "GL_INVALID_FRAMEBUFFER_OPERATION";
                    break;
                default:
                    error_str = "UNKNOWN_ERROR";
                    break;
            }
            RDE_CORE_ERROR("[OpenGL Error] ({0}): {1} at {2}:{3}", error, error_str, file, line);
        }
        return !error_found; // Return true if NO error was found
    }

    // The main macro definition.
#ifdef RDE_ENABLE_ASSERTS
#define GL_CHECK_ERROR() RDE_CORE_ASSERT(GLLogAnyError(__FILE__, __LINE__), "OpenGL Call Failed!")
#else
#define GL_CHECK_ERROR()
#endif
}
