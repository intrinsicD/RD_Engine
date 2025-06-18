// RDE_Project/applications/sandbox/SandboxApp.cpp

#include <glad/gl.h>
#include <Core/Application.h>
#include <Core/Events/KeyEvent.h>
#include <Core/EntryPoint.h>
#include <Core/FileIO.h>
#include <Core/Log.h>
#include <Core/Scene.h>
#include <Core/Entity.h>
#include <Core/Components.h>
#include <GlfwWindow.h> // We need the CONCRETE implementation
#include <Renderer/Shader.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Buffer.h>
#include <Renderer/Renderer2D.h>
#include <Renderer/RenderCommand.h>
#include <Renderer/OrthographicCameraController.h>
#include <imgui.h>

// The Sandbox is now a Layer, not an Application.
class SandboxLayer : public Layer {
public:
    SandboxLayer() : Layer("SandboxLayer"), m_camera_controller(1280.0f / 720.0f) {
        m_scene = std::make_shared<Scene>();

        // Create a square entity
        auto square = m_scene->CreateEntity("Green Square");
        square.AddComponent<TransformComponent>(glm::vec3(0.5f, 0.5f, 0.0f));
        square.AddComponent<SpriteRendererComponent>(glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    }

    void OnUpdate() override {
        m_camera_controller.OnUpdate(0.016f);

        RenderCommand::SetClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        RenderCommand::Clear();

        m_scene->OnUpdate(0.016f);

        // Render the scene
        Renderer2D::BeginScene(m_camera_controller.GetCamera());

        // New rendering path:
        auto view = m_scene->GetRegistry().view<TransformComponent, SpriteRendererComponent>();
        for (auto entity: view) {
            auto &transform = view.get<TransformComponent>(entity);
            auto &sprite = view.get<SpriteRendererComponent>(entity);
            Renderer2D::DrawQuad(transform.Translation, transform.Scale, sprite.Color);
        }

        Renderer2D::EndScene();

        // Display stats in ImGui
        auto stats = Renderer2D::GetStats();
        ImGui::Begin("Renderer2D Stats");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::End();
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
        m_camera_controller.OnEvent(e);
    }

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vertex_array;
    OrthographicCameraController m_camera_controller;
    std::shared_ptr<Texture2D> m_checkerboard_texture;
    std::shared_ptr<Scene> m_scene;
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
