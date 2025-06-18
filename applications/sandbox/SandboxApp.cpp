// RDE_Project/applications/sandbox/SandboxApp.cpp

#include <Core/Application.h>
#include <Core/Log.h>
#include <Core/Events/KeyEvent.h> // Include event definitions
#include <Renderer/Shader.h>
#include "GlfwWindow.h" // We need the CONCRETE implementation
#include "imgui.h" // Include imgui header

// The Sandbox is now a Layer, not an Application.
class SandboxLayer : public Layer {
public:
    SandboxLayer() : Layer("SandboxLayer") {
    }

    void OnUpdate() override {
        // Draw a simple window
        ImGui::Begin("Hello, RDE!");
        ImGui::Text("This is our first ImGui window.");
        ImGui::End();

        // You can also enable the full demo window to explore ImGui's features
        ImGui::ShowDemoWindow();
    }

    void OnEvent(Event &e) override {
        // RDE_TRACE("SandboxLayer Event: {0}", e.to_string());
        if (e.get_event_type() == EventType::KeyPressed) {
            KeyPressedEvent &ke = static_cast<KeyPressedEvent &>(e);
            if (ke.get_key_code() == 81) // 'Q' key
            {
                RDE_TRACE("'Q' key pressed, event handled by SandboxLayer.");
                e.handled = true; // This event is now consumed.
            }
        }
    }
};

class SandboxApp : public Application {
public:
    SandboxApp(std::unique_ptr<Window> window) : Application(std::move(window)) {
        RDE_INFO("Sandbox application created!");
        // Push our main layer onto the stack.
        PushLayer(new SandboxLayer());
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
