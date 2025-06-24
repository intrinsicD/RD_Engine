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
        SandboxApp(std::unique_ptr<IWindow> window, std::unique_ptr<IRenderer> renderer) : Application(std::move(window), std::move(renderer)) {
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
        WindowConfig window_config = {"SandboxApp", 1280, 720};

        auto window = GlfwOpenGLWindow::Create(window_config);
        RendererConfig renderer_config = {
                .window_handle = window->get_native_window(),
                .width = window_config.width,
                .height = window_config.height,
                .vsync = true,
                .api = RendererConfig::Api::OpenGL}; // Enable VSync
        return new SandboxApp(std::move(window), std::move(OpenGLRenderer::Create(renderer_config)));
    }
}
