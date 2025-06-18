// RDE_Project/applications/sandbox/SandboxApp.cpp

#include <Core/Application.h>
#include <Core/Events/KeyEvent.h>
#include <Core/EntryPoint.h>
#include <Core/Log.h>
#include <Core/Scene.h>
#include <Core/Entity.h>
#include <Core/Components.h>
#include <Core/FileIO.h>
#include <GlfwWindow.h> // We need the CONCRETE implementation
#include <Renderer/Shader.h>
#include <Renderer/VertexArray.h>
#include <Renderer/Renderer2D.h>
#include <Renderer/RenderCommand.h>
#include <Renderer/OrthographicCameraController.h>
#include <imgui.h>

namespace RDE {

// The Sandbox is now a Layer, not an Application.
    class SandboxLayer : public Layer {
    public:
        SandboxLayer() : Layer("SandboxLayer"), m_camera_controller(1280.0f / 720.0f) {
            m_scene = std::make_shared<Scene>();
            m_checkerboard_texture = RDE::Texture2D::Create(FileIO::get_path("assets/textures/Checkerboard.png"));

            // Create a square entity
            auto orange_square = m_scene->create_entity("Orange Square");
            orange_square.add_component<TransformComponent>(glm::vec3{0.5f, -0.5f, 0.0f});
            orange_square.add_component<SpriteRendererComponent>(glm::vec4{1.0f, 0.5f, 0.0f, 1.0f});

            auto textured_square = m_scene->create_entity("Textured Square");
            textured_square.add_component<TransformComponent>(glm::vec3{-0.5f, 0.5f, 0.0f});
            auto& textured_sprite = textured_square.add_component<SpriteRendererComponent>();
            textured_sprite.texture = m_checkerboard_texture;
            textured_sprite.tiling_factor = 2.0f;
        }

        void on_update() override {
            m_camera_controller.on_update(0.016f);

            RenderCommand::set_clear_color(0.1f, 0.1f, 0.15f, 1.0f);
            RenderCommand::clear();

            m_scene->on_update(0.016f);

            // Render the scene
            Renderer2D::begin_scene(m_camera_controller.GetCamera());

            // New rendering path:
            auto view = m_scene->get_registry().view<TransformComponent, SpriteRendererComponent>();
            for (auto entity : view)
            {
                // The view guarantees that both components exist for this entity.
                auto& transform = view.get<TransformComponent>(entity);
                auto& sprite = view.get<SpriteRendererComponent>(entity);

                // Submit a render command based on component data.
                Renderer2D::draw_quad(transform.get_transform(), sprite.texture, sprite.tiling_factor, sprite.color);
            }

            Renderer2D::end_scene();

            // Display stats in ImGui
            auto stats = Renderer2D::get_stats();
            ImGui::Begin("Renderer2D Stats");
            ImGui::Text("Draw Calls: %d", stats.draw_calls);
            ImGui::Text("Quads: %d", stats.quad_count);
            ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
            ImGui::Text("Indices: %d", stats.get_total_index_count());
            ImGui::End();
        }

        void on_event(Event &e) override {
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
        Entity m_selected_entity;
    };

    class SandboxApp : public Application {
    public:
        SandboxApp(std::unique_ptr<Window> window) : Application(std::move(window)) {
            RDE_INFO("Sandbox application created!");
            // Push our main layer onto the stack.
            push_layer(new SandboxLayer());
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