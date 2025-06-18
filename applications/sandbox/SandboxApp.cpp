// RDE_Project/applications/sandbox/SandboxApp.cpp

#include "Application.h"
#include "Events/KeyEvent.h"
#include "Log.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "FileIO.h"
#include "src/GlfwWindow.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Renderer2D.h"
#include "RenderCommand.h"
#include "OrthographicCameraController.h"
#include "Serialization.h"
#include "SceneSerializer.h"
#include "EntryPoint.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include <yaml-cpp/yaml.h> // We need this for the lambda bodies

namespace RDE {
    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string &name, Entity entity, UIFunction ui_function) {
        const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

        if (entity.has_component<T>()) {
            auto &component = entity.get_component<T>();
            ImVec2 content_region_available = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), tree_node_flags, name.c_str());
            ImGui::PopStyleVar();

            ImGui::SameLine(content_region_available.x - line_height * 0.5f);
            if (ImGui::Button("+", ImVec2{line_height, line_height})) {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool remove_component = false;
            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove component"))
                    remove_component = true;

                ImGui::EndPopup();
            }

            if (open) {
                ui_function(component);
                ImGui::TreePop();
            }

            if (remove_component)
                entity.remove_component<T>();
        }
    }

    // The Sandbox is now a Layer, not an Application.
    class SandboxLayer : public Layer {
    public:
        SandboxLayer() : Layer("SandboxLayer"), m_camera_controller(1280.0f / 720.0f), m_selected_entity() {
            m_scene = std::make_shared<Scene>();
            m_scene_serializer = std::make_shared<SceneSerializer>(m_scene);
            m_checkerboard_texture = RDE::Texture2D::Create(FileIO::get_path("assets/textures/Checkerboard.png"));

            // Create a square entity
            auto orange_square = m_scene->create_entity("Orange Square");
            orange_square.add_component<TransformComponent>(glm::vec3{0.5f, -0.5f, 0.0f});
            orange_square.add_component<SpriteRendererComponent>(glm::vec4{1.0f, 0.5f, 0.0f, 1.0f});

            auto textured_square = m_scene->create_entity("Textured Square");
            textured_square.add_component<TransformComponent>(glm::vec3{-0.5f, 0.5f, 0.0f});
            auto &textured_sprite = textured_square.add_component<SpriteRendererComponent>();
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
            for (auto entity: view) {
                // The view guarantees that both components exist for this entity.
                auto &transform = view.get<TransformComponent>(entity);
                auto &sprite = view.get<SpriteRendererComponent>(entity);
                transform.rotation.z += 0.01f; // Rotate the entity slightly for demonstration
                // Submit a render command based on component data.
                Renderer2D::draw_quad(transform.get_transform(), sprite.texture, sprite.tiling_factor, sprite.color);
            }

            Renderer2D::end_scene();
        }

        void on_gui_render() override {
            draw_scene_hierarchy_panel();
            draw_properties_panel();

            // Display stats in ImGui
            auto stats = Renderer2D::get_stats();
            ImGui::Begin("Renderer2D Stats");
            ImGui::Text("Draw Calls: %d", stats.draw_calls);
            ImGui::Text("Quads: %d", stats.quad_count);
            ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
            ImGui::Text("Indices: %d", stats.get_total_index_count());
            ImGui::End();

            if (ImGui::MenuItem("Save Scene As...")) {
               save_scene();
            }

            if (ImGui::MenuItem("Open Scene...")) {
                load_scene();
            }
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

        void draw_scene_hierarchy_panel() {
            ImGui::Begin("Scene Hierarchy");

            // Existing loop to display each entity
            m_scene->get_registry().view<entt::entity>().each([&](auto entity_handle) {
                Entity entity{entity_handle, m_scene.get()};
                auto &tag = entity.get_component<TagComponent>().tag;

                // Use a unique ID for the TreeNode. The entity handle is perfect for this.
                ImGuiTreeNodeFlags flags = ((m_selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                           ImGuiTreeNodeFlags_OpenOnArrow;
                flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // Makes the node span the full width

                // We use TreeNodeEx to have more control and prepare for parenting.
                bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, tag.c_str());

                // Handle selection
                if (ImGui::IsItemClicked()) {
                    m_selected_entity = entity;
                }

                // Context menu for deleting the specific entity (right-click on item)
                bool entity_deleted = false;
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete Entity"))
                        entity_deleted = true;

                    ImGui::EndPopup();
                }

                if (opened) {
                    // This is where child entities would be rendered in a full hierarchy
                    ImGui::TreePop();
                }

                // Defer deletion to the end of the loop to avoid iterator invalidation issues.
                if (entity_deleted) {
                    m_scene->destroy_entity(entity);
                    if (m_selected_entity == entity)
                        m_selected_entity = {}; // Reset selection if deleted
                }
            });

            // Deselect if clicking on empty space.
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_selected_entity = {};

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // The explicit "Create Entity" button, always accessible.
            // This is the robust replacement for the flawed BeginPopupContextWindow call.
            if (ImGui::Button("Create Entity")) {
                m_selected_entity = m_scene->create_entity("New Entity");
            }

            ImGui::End();
        }

        void draw_properties_panel() {
            ImGui::Begin("Properties");

            // Only draw properties if an entity is actually selected
            if (m_selected_entity) {
                // Tag Component is special, we won't make it removable.
                if (m_selected_entity.has_component<TagComponent>()) {
                    auto &tag = m_selected_entity.get_component<TagComponent>().tag;
                    char buffer[256];
                    memset(buffer, 0, sizeof(buffer));
                    strncpy(buffer, tag.c_str(), sizeof(buffer) - 1); // Copy tag to buffer
                    if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                        tag = std::string(buffer);
                    }
                }

                // Use our new template helper for TransformComponent
                DrawComponent<TransformComponent>("Transform", m_selected_entity, [](auto &component) {
                    ImGui::DragFloat3("Translation", glm::value_ptr(component.translation), 0.1f);
                    glm::vec3 rotation_degrees = glm::degrees(component.rotation);
                    if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation_degrees), 1.0f)) {
                        component.rotation.z = glm::radians(rotation_degrees.z);
                    }
                    ImGui::DragFloat3("Scale", glm::value_ptr(component.scale), 0.1f);
                });

                // And for SpriteRendererComponent
                DrawComponent<SpriteRendererComponent>("Sprite Renderer", m_selected_entity, [](auto &component) {
                    ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
                });

                // "Add Component" button
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Add Component"))
                    ImGui::OpenPopup("AddComponent");

                if (ImGui::BeginPopup("AddComponent")) {
                    if (ImGui::MenuItem("Sprite Renderer")) {
                        if (!m_selected_entity.has_component<SpriteRendererComponent>())
                            m_selected_entity.add_component<SpriteRendererComponent>();
                        else
                            // Optional: Log a warning that component already exists
                            // RDE_CORE_WARN("Entity already has a Sprite Renderer Component!");
                            ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Transform")) {
                        if (!m_selected_entity.has_component<TransformComponent>())
                            m_selected_entity.add_component<TransformComponent>();
                        else
                            // Optional: Log a warning that component already exists
                            // RDE_CORE_WARN("Entity already has a Transform Component!");
                            ImGui::CloseCurrentPopup();
                    }

                    // Add other components here in the future
                    // if (ImGui::MenuItem("Camera")) { ... }

                    ImGui::EndPopup();
                }
            }

            ImGui::End();
        }

        void save_scene(const std::string &filepath = "assets/scenes/MyScene.rde") {
            m_scene_serializer->serialize(filepath,
                    // This is our serialization lambda (the "hook")
                    [](YAML::Emitter& out, Entity entity) {
                        if (entity.has_component<SpriteRendererComponent>()) {
                            out << YAML::Key << "SpriteRendererComponent";
                            out << YAML::BeginMap; // SpriteRendererComponent

                            auto& src = entity.get_component<SpriteRendererComponent>();
                            out << YAML::Key << "Color" << YAML::Value << src.color;
                            if (src.texture)
                                out << YAML::Key << "TexturePath" << YAML::Value << src.texture->get_path();
                            out << YAML::Key << "TilingFactor" << YAML::Value << src.tiling_factor;

                            out << YAML::EndMap; // SpriteRendererComponent
                        }
                    });
        }

        void load_scene(const std::string &filepath = "assets/scenes/MyScene.rde") {
            m_selected_entity = {};
            m_scene_serializer->deserialize(filepath,
                // This is our deserialization lambda (the "hook")
                [](const YAML::Node& entity_node, Entity entity) {
                    auto sprite_component = entity_node["SpriteRendererComponent"];
                    if (sprite_component) {
                        auto& src = entity.add_component<SpriteRendererComponent>();
                        src.color = sprite_component["Color"].as<glm::vec4>();
                        if (sprite_component["TexturePath"]) {
                            std::string path = sprite_component["TexturePath"].as<std::string>();
                            src.texture = RDE::Texture2D::Create(path);
                        }
                        src.tiling_factor = sprite_component["TilingFactor"].as<float>();
                    }
                });
        }

    private:
        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<VertexArray> m_vertex_array;
        OrthographicCameraController m_camera_controller;
        std::shared_ptr<Texture2D> m_checkerboard_texture;
        std::shared_ptr<Scene> m_scene;
        std::shared_ptr<SceneSerializer> m_scene_serializer;
        Entity m_selected_entity;
    };

    class SandboxApp : public Application {
    public:
        SandboxApp(std::unique_ptr<Window> window) : Application(std::move(window)) {
            RDE_INFO("Sandbox application created!");
            // Push our main layer onto the stack.
            push_layer(std::make_unique<SandboxLayer>());
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
