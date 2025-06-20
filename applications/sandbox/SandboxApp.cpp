// RDE_Project/applications/sandbox/SandboxApp.cpp

#include "Application.h"
#include "Events/KeyEvent.h"
#include "Log.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/CameraComponent.h"
#include "EntityComponents/TransformComponent.h"
#include "Components/SpriteRendererComponent.h"
#include "EntityComponents/TagComponent.h"
#include "Components/MeshComponent.h"
#include "PerspectiveCamera.h"
#include "FileIO.h"
#include "src/GlfwWindow.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "OrthographicCameraController.h"
#include "Serialization.h"
#include "SceneSerializer.h"
#include "EntryPoint.h"
#include "EditorCamera.h"
#include "src/ComponentUIRegistry.h"
#include "ISystem.h"
#include "src/EditorLayer.h"
#include "src/SandboxLayer.h"


#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <yaml-cpp/yaml.h> // We need this for the lambda bodies

#include "Components/ArcballControllerComponent.h"

namespace RDE {
    // The Sandbox is now a Layer, not an Application.
    namespace UNUSED {
        class SandboxLayer : public Layer {
        public:
            SandboxLayer() : Layer("SandboxLayer"), /*m_camera_controller(1280.0f / 720.0f),*/ m_selected_entity(),
                             m_editor_camera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f) {
                m_scene = std::make_shared<Scene>();
                m_scene_serializer = std::make_shared<SceneSerializer>(m_scene);
                m_checkerboard_texture = RDE::Texture2D::Create(FileIO::GetPath("assets/textures/Checkerboard.png"));

                m_3d_shader = Shader::CreateFromFile(FileIO::GetPath("assets/shaders/Simple3D.vert"),
                                                     FileIO::GetPath("assets/shaders/Simple3D.frag"));

                m_cube_entity = m_scene->create_entity("Textured Cube");
                auto &mc = m_cube_entity.add_component<MeshComponent>(Mesh::CreateCube());
                mc.texture = m_checkerboard_texture; // Assign the texture
                m_cube_entity.add_component<TransformComponent>();
                RenderCommand::SetClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            }

            void on_update() override {
                m_editor_camera.on_update(0.016f);
                RenderCommand::Clear();

                // 1. Set up the scene's camera matrices
                Renderer::BeginScene(m_editor_camera.get_view_projection());

                // 2. Execute the 3D Pass
                Renderer::Begin3DPass();
                {
                    // --- This is much cleaner ---
                    // 1. Set the shader and all scene-level data for the pass
                    Renderer3D::SetShaderAndSceneUniforms(m_3d_shader);

                    // 2. Submit all mesh entities
                    auto view = m_scene->get_registry().view<TransformComponent, MeshComponent>();
                    for (auto entity: view) {
                        auto [transform, mesh_comp] = view.get<TransformComponent, MeshComponent>(entity);
                        if (mesh_comp.mesh) {
                            // Set per-entity material properties on the already-bound shader
                            bool use_texture = mesh_comp.texture != nullptr;
                            m_3d_shader->set_int("u_UseTexture", use_texture);

                            if (use_texture) {
                                mesh_comp.texture->bind(0);
                                m_3d_shader->set_int("u_TextureSampler", 0);
                            } else {
                                m_3d_shader->set_float("u_Color", mesh_comp.color);
                            }

                            // Submit only the per-entity data
                            Renderer3D::Submit(mesh_comp.mesh->get_vertex_array(), transform.get_transform());
                        }
                    }
                }
                Renderer::End3DPass();

                // 3. Execute the 2D Pass (for UI, etc.)
                Renderer::Begin2DPass();
                {
                    // Example: Draw a UI element if needed.
                    // Renderer::DrawScreenSpaceQuad({100, 100}, {50, 50}, {0.2, 0.8, 0.3, 1.0});
                }
                Renderer::End2DPass();

                // 4. Finalize the frame
                Renderer::EndScene();
            }

            void on_gui_render() override {

            }

            bool on_window_resize(WindowResizeEvent &e) {
                float width = (float) e.get_width();
                float height = (float) e.get_height();

                // It's good practice to check for zero dimensions here as well,
                // even though the Application class already does.
                if (width == 0 || height == 0) return false;

                // 1. Update the Renderer's viewport
                Renderer::OnWindowResize((uint32_t) width, (uint32_t) height);

                m_editor_camera.set_viewport_size(width, height);

                // Return false to indicate the event can be processed by other layers if necessary.
                return false;
            }

            void on_event(Event &e) override {
                m_editor_camera.on_event(e);

                EventDispatcher dispatcher(e);
                dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(SandboxLayer::on_window_resize));

                // RDE_TRACE("SandboxLayer Event: {0}", e.to_string());
                if (e.get_event_type() == EventType::KeyPressed) {
                    KeyPressedEvent &ke = static_cast<KeyPressedEvent &>(e);
                    if (ke.get_key_code() == 81) // 'Q' key
                    {
                        RDE_TRACE("'Q' key pressed, event handled by SandboxLayer.");
                        e.handled = true; // This event is now consumed.
                    }
                }
                /*     m_camera_controller.on_event(e);*/
            }


        private:
            std::shared_ptr<Shader> m_shader;
            std::shared_ptr<Shader> m_3d_shader;
            std::shared_ptr<VertexArray> m_vertex_array;
            /*        OrthographicCameraController m_camera_controller;*/
            std::shared_ptr<Texture2D> m_checkerboard_texture;
            std::shared_ptr<SceneSerializer> m_scene_serializer;
            Entity m_selected_entity;
            Entity m_camera_entity;
            Entity m_cube_entity;
            EditorCamera m_editor_camera; // Optional: For 3D camera controls
            std::shared_ptr<Scene> m_scene;
            std::vector<std::shared_ptr<ISystem>> m_systems; // Optional: For systems in the scene
        };
    }

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
