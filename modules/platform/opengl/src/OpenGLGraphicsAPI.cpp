// RDE_Project/modules/platform/opengl/OpenGLGraphicsAPI.cpp

#include "OpenGLGraphicsAPI.h"
#include "OpenGLDebug.h"
#include "Base.h"
#include "Log.h"

#include <glad/gl.h>

namespace RDE {
// This global static variable determines which API is currently active.
// We will set it when the renderer is initialized.
    GraphicsAPI::API GraphicsAPI::s_api = GraphicsAPI::API::OpenGL;

    std::unique_ptr<GraphicsAPI> GraphicsAPI::Create() {
        switch (s_api) {
            case GraphicsAPI::API::None:
                RDE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
                return nullptr;
            case GraphicsAPI::API::OpenGL:
                return std::make_unique<OpenGLGraphicsAPI>();
        }
        RDE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
        return nullptr;
    }

    void OpenGLGraphicsAPI::init() {
        // In the future, this will enable blending, depth testing, etc.
        RDE_CORE_INFO("OpenGLGraphicsAPI::Init - Enabling depth testing (placeholder)");
        // GL_CALL(glEnable(GL_DEPTH_TEST));
    }

    void OpenGLGraphicsAPI::set_clear_color(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        GL_CHECK_ERROR();
    }

    void OpenGLGraphicsAPI::set_depth_test(bool enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        GL_CHECK_ERROR();
    }

    void OpenGLGraphicsAPI::set_blending(bool enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
        GL_CHECK_ERROR();
    }

    void OpenGLGraphicsAPI::clear() {
        // For now, we clear color. Later, we'll also clear depth and stencil buffers.
        glClear(GL_COLOR_BUFFER_BIT);
        GL_CHECK_ERROR();
    }

    void OpenGLGraphicsAPI::draw_indexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount) {
        RDE_CORE_ASSERT(vertexArray->get_index_buffer(), "VertexArray has no index buffer!");
        uint32_t count = indexCount ? indexCount : vertexArray->get_index_buffer()->get_count();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERROR();
    }
}