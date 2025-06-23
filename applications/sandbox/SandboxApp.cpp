// RDE_Project/applications/sandbox/SandboxApp.cpp

#include "Application.h"
#include "Log.h"
#include "src/GlfwOpenGLWindow.h"
#include "src/EditorLayer.h"
#include "src/SandboxLayer.h"
#include "OpenGLRenderer.h"
#include "EntryPoint.h"

namespace RDE {
    class SandboxApp : public Application {
    public:
        SandboxApp(const Config::WindowConfig &window_config, const Config::RendererConfig &renderer_config) : Application(window_config, renderer_config) {
            RDE_INFO("Sandbox application created!");
            // Push our main layer onto the stack.
            auto sandbox_layer = std::make_shared<SandboxLayer>();
            push_layer(sandbox_layer);
            auto *scene = sandbox_layer->get_scene();

            auto editor_layer = std::make_shared<EditorLayer>(scene);
            push_layer(editor_layer);
        }

        ~SandboxApp() {
            RDE_INFO("Sandbox application destroyed!");
        }
    };

    // This is the function the engine's entry point will call.
    // We return a new instance of our Sandbox application.
    Application *CreateApplication() {
        Config::WindowConfig window_config = {"SandboxApp", 1280, 720};
        Config::RendererConfig renderer_config = {Config::RendererAPI::OpenGL_4_5, true}; // Enable VSync
        auto window = GlfwOpenGLWindow::Create(window_config);
        auto renderer = OpenGLRenderer::Create(window_config, renderer_config);
        return new SandboxApp(window_config, renderer_config);
    }
}
