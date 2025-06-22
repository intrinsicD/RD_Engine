#include "EditorLayer.h"

#include "ui/UIUtils.h"
#include "ui/ComponentUIRegistry.h"

#include "EntityComponents/TransformComponent.h"
#include "EntityComponents/MeshComponent.h"
#include "EntityComponents/SpriteRendererComponent.h"
#include "EntityComponents/ArcballControllerComponent.h"
#include "EntityComponents/CameraComponent.h"
#include "EntityComponents/TagComponent.h"

#include "YamlUtils.h"
#include "SceneSerializer.h"
#include "FileIO.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <yaml-cpp/yaml.h> // For YAML serialization/deserialization

#include "EntityComponents/PrimaryCameraTag.h"

// ... all your component and ImGui includes ...

namespace RDE {
    EditorLayer::EditorLayer() : Layer("EditorLayer") {
    }

    void EditorLayer::set_context(const std::shared_ptr<Scene> &scene) {
        m_scene = scene;
        m_selected_entity = {}; // Reset selection when context changes
    }

    void EditorLayer::on_attach() {
        register_component_uis();
    }

    void EditorLayer::on_gui_render() {
        // ... Dockspace setup code ...
        draw_scene_hierarchy_panel();
        draw_properties_panel();
        // ... End dockspace ...
    }

    // Move draw_scene_hierarchy_panel and draw_properties_panel here.
    // They will now use m_context_scene and m_selected_entity.
    // The logic inside them remains the same as what you wrote.
    void EditorLayer::draw_scene_hierarchy_panel() {
        ImGui::Begin("Scene Hierarchy");
        if (ImGui::MenuItem("Save Scene As...")) {
            save_scene();
        }

        if (ImGui::MenuItem("Open Scene...")) {
            load_scene();
        }

        // Existing loop to display each entity
        m_scene->get_registry().view<entt::entity>().each([&](auto entity_handle) {
            Entity entity{entity_handle, m_scene.get()};
            auto &tag = entity.get_component<TagComponent>().tag;

            // Use a unique ID for the TreeNode. The entity handle is perfect for this.
            ImGuiTreeNodeFlags flags = ((m_selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                       ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // Makes the node span the full width

            // We use TreeNodeEx to have more control and prepare for parenting.
            bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, "%s", tag.c_str());

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

    void EditorLayer::draw_properties_panel() {
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

            ComponentUIRegistry::Draw<TransformComponent>(m_scene.get(), m_selected_entity);
            ComponentUIRegistry::Draw<MeshComponent>(m_scene.get(), m_selected_entity);
            ComponentUIRegistry::Draw<SpriteRendererComponent>(m_scene.get(), m_selected_entity);
            ComponentUIRegistry::Draw<ArcballControllerComponent>(m_scene.get(), m_selected_entity);
            ComponentUIRegistry::Draw<CameraComponent>(m_scene.get(), m_selected_entity);

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
                if (ImGui::MenuItem("Mesh Renderer")) {
                    if (!m_selected_entity.has_component<MeshComponent>())
                        m_selected_entity.add_component<MeshComponent>();
                    else
                        // Optional: Log a warning that component already exists
                        // RDE_CORE_WARN("Entity already has a Mesh Renderer Component!");
                        ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Arcball Controller")) {
                    if (!m_selected_entity.has_component<ArcballControllerComponent>())
                        m_selected_entity.add_component<ArcballControllerComponent>();
                    else
                        // Optional: Log a warning that component already exists
                        // RDE_CORE_WARN("Entity already has an Arcball Controller Component!");
                        ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Camera")) {
                    if (!m_selected_entity.has_component<CameraComponent>())
                        m_selected_entity.add_component<CameraComponent>();
                    else
                        // Optional: Log a warning that component already exists
                        // RDE_CORE_WARN("Entity already has a Camera Component!");
                        ImGui::CloseCurrentPopup();
                }

                // Add other components here in the future
                // if (ImGui::MenuItem("FutureComponent")) { ... }

                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void EditorLayer::register_component_uis() {
        ComponentUIRegistry::RegisterComponent<TransformComponent>("Transform", [](Entity entity) {
            UI::DrawComponent<TransformComponent>("Transform", entity, [](auto &component) {
                ImGui::DragFloat3("Translation", glm::value_ptr(component.position), 0.1f);
                glm::vec3 rotation_degrees = glm::degrees(component.rotation);
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation_degrees), 1.0f)) {
                    component.rotation.z = glm::radians(rotation_degrees.z);
                }
                ImGui::DragFloat3("Scale", glm::value_ptr(component.scale), 0.1f);
            });
        });

        ComponentUIRegistry::RegisterComponent<SpriteRendererComponent>("Sprite Renderer", [](Entity entity) {
            UI::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto &component) {
                ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            });
        });

        ComponentUIRegistry::RegisterComponent<MeshComponent>("Mesh Renderer", [](Entity entity) {
            UI::DrawComponent<MeshComponent>("Mesh Renderer", entity, [](auto &component) {
                ImGui::ColorEdit4("Color", glm::value_ptr(component.color));

                ImGui::Text("Texture: %s", component.texture ? component.texture->get_path().c_str() : "None");
                if (ImGui::Button("Select Mesh")) {
                    // Open a file dialog to select a mesh asset
                    // This is a placeholder, actual implementation would depend on your asset management system
                    // For now, we just log the action
                    RDE_CORE_INFO("Open mesh selection dialog (not implemented)");
                }
                if (ImGui::Button("Select Texture")) {
                    // Open a file dialog to select a texture asset
                    // This is a placeholder, actual implementation would depend on your asset management system
                    // For now, we just log the action
                    RDE_CORE_INFO("Open texture selection dialog (not implemented)");
                }

                // Later: Add a field for the mesh asset, texture asset, etc.
            });
        });
        ComponentUIRegistry::RegisterComponent<ArcballControllerComponent>(
            "ArcBall Controller", [](Entity entity) {
                UI::DrawComponent<ArcballControllerComponent>("ArcBall Controller", entity, [](auto &component) {
                    ImGui::InputFloat3("Focal Point", glm::value_ptr(component.focal_point));
                    ImGui::DragFloat("Distance", &component.distance, 0.1f, 0.1f, 100.0f);
                    ImGui::Text("width %i", component.width);
                    ImGui::Text("height %i", component.height);

                    ImGui::Text("last_point_2d %f, %f", component.last_point_2d.x, component.last_point_2d.y);
                    ImGui::Text("last_point_3d %f, %f, %f", component.last_point_3d.x,
                                component.last_point_3d.y, component.last_point_3d.z);
                    ImGui::Text("last_point_ok %i", component.last_point_ok);
                });
            });
        ComponentUIRegistry::RegisterComponent<CameraComponent>(
            "Camera", [&](Entity entity) {
                UI::DrawComponent<CameraComponent>("Camera", entity, [&](auto &component) {
                    bool is_primary = m_scene->get_registry().all_of<PrimaryCameraTag>(entity);
                    ImGui::Checkbox("Primary Camera", &is_primary);

                    const auto &projection_matrix = component.projection_matrix;
                    ImGui::Text("Projection Matrix:");
                    std::stringstream ss_proj;
                    ss_proj << glm::to_string(projection_matrix);
                    ImGui::Text("  %s", ss_proj.str().c_str());
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    const auto &view_matrix = component.view_matrix;
                    ImGui::Text("View Matrix:");
                    std::stringstream ss_view;
                    ss_view << glm::to_string(view_matrix);
                    ImGui::Text("  %s", ss_view.str().c_str());
                });
            });
    }

    void EditorLayer::save_scene(const std::string &filepath) {
        SceneSerializer serializer(m_scene);
        serializer.serialize(FileIO::GetPath(filepath),
                             // This is our serialization lambda (the "hook")
                             [](YAML::Emitter &out, Entity entity) {
                                 if (entity.has_component<SpriteRendererComponent>()) {
                                     out << YAML::Key << "SpriteRendererComponent";
                                     out << YAML::BeginMap; // SpriteRendererComponent

                                     auto &src = entity.get_component<SpriteRendererComponent>();
                                     out << YAML::Key << "Color" << YAML::Value << src.color;
                                     if (src.texture)
                                         out << YAML::Key << "TexturePath" << YAML::Value
                                                 << src.texture->get_path();
                                     out << YAML::Key << "TilingFactor" << YAML::Value
                                             << src.tiling_factor;

                                     out << YAML::EndMap; // SpriteRendererComponent
                                 }
                             });
    }

    void EditorLayer::load_scene(const std::string &filepath) {
        m_selected_entity = {};
        SceneSerializer serializer(m_scene);
        serializer.deserialize(FileIO::GetPath(filepath),
                               // This is our deserialization lambda (the "hook")
                               [](const YAML::Node &entity_node, Entity entity) {
                                   auto sprite_component = entity_node["SpriteRendererComponent"];
                                   if (sprite_component) {
                                       auto &src = entity.add_component<SpriteRendererComponent>();
                                       src.color = sprite_component["Color"].as<glm::vec4>();
                                       if (sprite_component["TexturePath"]) {
                                           std::string path = sprite_component["TexturePath"].as<
                                               std::string>();
                                           src.texture = RDE::Texture2D::Create(path);
                                       }
                                       src.tiling_factor = sprite_component["TilingFactor"].as<float>();
                                   }
                               });
    }
}
