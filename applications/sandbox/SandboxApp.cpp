// RDE_Project/applications/sandbox/SandboxApp.cpp

#include "Application.h"
#include "Log.h"
#include "src/GlfwWindow.h"
#include "src/EditorLayer.h"
#include "src/SandboxLayer.h"
#include "EntryPoint.h"

namespace RDE {
    class SandboxApp : public Application {
    public:
        SandboxApp(std::unique_ptr<Window> window) : Application(std::move(window)) {
            RDE_INFO("Sandbox application created!");
            // Push our main layer onto the stack.
            auto sandbox_layer = std::make_shared<SandboxLayer>();
            push_layer(sandbox_layer);

            auto editor_layer = std::make_shared<EditorLayer>();
            editor_layer->set_context(sandbox_layer->get_scene());
            push_layer(editor_layer);
        }

        ~SandboxApp() {
            RDE_INFO("Sandbox application destroyed!");
        }
    };

    // This is the function the engine's entry point will call.
    // We return a new instance of our Sandbox application.
    Application *CreateApplication() {
        WindowProps props = {"SandboxApp", 1280, 720};
        return new SandboxApp(std::move(GlfwWindow::Create(props)));
    }
}
