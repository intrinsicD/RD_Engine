// RDE_Project/applications/sandbox/SandboxApp.cpp

#include <glad/gl.h>
#include <Core/Application.h>
#include <Core/Log.h>
#include <Core/Events/KeyEvent.h>
#include <Core/EntryPoint.h>
#include <Core/FileIO.h>
#include <Renderer/Shader.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Buffer.h>
#include <GlfwWindow.h> // We need the CONCRETE implementation
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/OrthographicCameraController.h"
#include <imgui.h>

// The Sandbox is now a Layer, not an Application.
class SandboxLayer : public Layer {
public:
    SandboxLayer() : Layer("SandboxLayer"), m_camera_controller(1280.0f / 720.0f) {
        // --- Triangle Setup ---
        m_vertex_array = VertexArray::Create();

        float vertices[3 * 3] = {
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.0f, 0.5f, 0.0f
        };
        auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        BufferLayout layout = {
                {ShaderDataType::Float3, "a_Position"}
        };
        vertexBuffer->SetLayout(layout);
        m_vertex_array->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = {0, 1, 2};
        auto indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        m_vertex_array->SetIndexBuffer(indexBuffer);

        // --- Shader Setup ---
        auto vertPath = FileIO::GetPath("assets/shaders/triangle.vert");
        auto fragPath = FileIO::GetPath("assets/shaders/triangle.frag");
        m_shader = Shader::CreateFromFile(vertPath, fragPath);

        m_checkerboard_texture = Texture2D::Create(FileIO::GetPath("assets/textures/Checkerboard.png"));
    }

    void OnUpdate() override {
        m_camera_controller.OnUpdate(0.016f);

        RenderCommand::SetClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        RenderCommand::Clear();

        Renderer2D::BeginScene(m_camera_controller.GetCamera());

        // Use the new rotated quad functions
        Renderer2D::DrawRotatedQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(45.0f), {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, {0.2f, 0.3f, 0.8f, 1.0f});
        Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, glm::radians(-10.0f), m_checkerboard_texture, 10.0f);

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
