#include "EditorLayer.h"
#include "Application.h"
#include "assets/AssetManager.h"

#include "ui/UIUtils.h"
#include "ui/ComponentUIRegistry.h"

#include "components/AABBComponent.h"
#include "components/AnimationComponent.h"
#include "components/BoundingSphereComponent.h"
#include "components/CameraComponent.h"
#include "components/ColliderComponent.h"
#include "components/IsPrimaryTag.h"
#include "components/MaterialComponent.h"
#include "components/NameTagComponent.h"
#include "components/RenderableComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/SkeletonComponent.h"
#include "components/TransformComponent.h"

#include "utils/FileIOUtils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <yaml-cpp/yaml.h> // For YAML serialization/deserialization

namespace RDE {
    EditorLayer::EditorLayer(Scene *scene) : ILayer("EditorLayer"), m_scene(scene) {
        m_selected_entity = {}; // Reset selection when context changes
    }

    void EditorLayer::on_attach() {
        register_component_uis();
    }

    void EditorLayer::on_gui_render() {
        // Render Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save As...")) {
                save_scene();
            }
            if (ImGui::MenuItem("Open...")) {
                IGFD::FileDialogConfig config;
                config.path = ".";
                config.path = "/home/alex/Dropbox/Work/Datasets";
                ImGuiFileDialog::Instance()->OpenDialog("Load Geometry", "Choose File", ".obj,.off,.stl,.ply",
                                                        config);
            }
            ImGui::EndMenu();
        }

        if (ImGuiFileDialog::Instance()->Display("Load Geometry")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                auto path = ImGuiFileDialog::Instance()->GetFilePathName();
                create_renderable_entity_from_asset(path);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // ... Dockspace setup code ...
        draw_scene_hierarchy_panel();
        draw_properties_panel();
        // ... End dockspace ...
    }

    void EditorLayer::on_event(RDE::Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowFileDropEvent>([this](WindowFileDropEvent& event) {
            for(const auto& path : event.get_files()) {
                auto ext = FileIO::GetFileExtension(path);
                create_renderable_entity_from_asset(path);
            }
            return true; // We handled the event.
        });
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
            IGFD::FileDialogConfig config;
            config.path = ".";
            config.path = "/home/alex/Dropbox/Work/Datasets";
            ImGuiFileDialog::Instance()->OpenDialog("Load Geometry", "Choose File", ".obj,.off,.stl,.ply",
                                                    config);
        }

        // Existing loop to display each entity
        m_scene->get_registry().view<entt::entity>().each([&](auto entity_handle) {
            Entity entity{entity_handle, m_scene};
            auto &name = entity.get_component<Components::NameTagComponent>().name;

            // Use a unique ID for the TreeNode. The entity handle is perfect for this.
            ImGuiTreeNodeFlags flags = ((m_selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                       ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // Makes the node span the full width

            // We use TreeNodeEx to have more control and prepare for parenting.
            bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, "%s", name.c_str());

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
            if (m_selected_entity.has_component<Components::NameTagComponent>()) {
                auto &tag = m_selected_entity.get_component<Components::NameTagComponent>().name;
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strncpy(buffer, tag.c_str(), sizeof(buffer) - 1); // Copy tag to buffer
                if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                    tag = std::string(buffer);
                }
            }

            ComponentUIRegistry::Draw<Components::AABBLocalComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::AABBWorldComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::AnimationComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::BoundingSphereLocalComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::BoundingSphereWorldComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::CameraComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::ColliderComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::MaterialComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::RenderableComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::RigidBodyComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::SkeletonComponent>(m_scene, m_selected_entity);
            ComponentUIRegistry::Draw<Components::Transform>(m_scene, m_selected_entity);

            // "Add Component" button
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");

            if (ImGui::BeginPopup("AddComponent")) {
                UI::DrawAddComponentPopupMenuItem<Components::AABBLocalComponent>("AABB local", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::AABBWorldComponent>("AABB world", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::AnimationComponent>("Animation", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::BoundingSphereLocalComponent>("Bounding Sphere Local",
                                                                                            m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::BoundingSphereWorldComponent>("Bounding Sphere World",
                                                                                            m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::CameraComponent>("Camera", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::ColliderComponent>("Collider", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::MaterialComponent>("Material", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::RenderableComponent>("Renderable", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::RigidBodyComponent>("Rigid Body", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::SkeletonComponent>("Skeleton", m_selected_entity);
                UI::DrawAddComponentPopupMenuItem<Components::Transform>("Transform", m_selected_entity);
                // Add other components here in the future
                // if (ImGui::MenuItem("FutureComponent")) { ... }

                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void EditorLayer::register_component_uis() {
        ComponentUIRegistry::RegisterComponent<Components::AABBLocalComponent>("AABB Local", [](Entity entity) {
            UI::DrawComponent<Components::AABBLocalComponent>("AABB Local", entity, [](auto &component) {
                ImGui::Text("Min: %s", glm::to_string(component.min).c_str());
                ImGui::Text("Max: %s", glm::to_string(component.max).c_str());
            });
        });
        ComponentUIRegistry::RegisterComponent<Components::AABBWorldComponent>("AABB World", [](Entity entity) {
            UI::DrawComponent<Components::AABBWorldComponent>("AABB World", entity, [](auto &component) {
                ImGui::Text("Min: %s", glm::to_string(component.min).c_str());
                ImGui::Text("Max: %s", glm::to_string(component.max).c_str());
            });
        });
        ComponentUIRegistry::RegisterComponent<Components::AnimationComponent>("Animation", [](Entity entity) {
            UI::DrawComponent<Components::AnimationComponent>("Animation", entity, [](auto &component) {
                ImGui::Text("Animation Handle: %i", component.animation_handle.get_asset_id());
                ImGui::Text("Current Time: %f", component.current_time);
                ImGui::Text("Is Looping: %i", component.is_looping);
                ImGui::DragFloat("Playback Speed", &component.playback_speed, 0.01f, 0.01f, 10.0f);
            });
        });
        ComponentUIRegistry::RegisterComponent<Components::BoundingSphereLocalComponent>("Bounding Sphere Local",
                                                                                         [](Entity entity) {
                                                                                             UI::DrawComponent<Components::BoundingSphereLocalComponent>(
                                                                                                     "Bounding Sphere Local",
                                                                                                     entity,
                                                                                                     [](auto &component) {
                                                                                                         ImGui::Text(
                                                                                                                 "Center: %s",
                                                                                                                 glm::to_string(
                                                                                                                         component.center).c_str());
                                                                                                         ImGui::DragFloat(
                                                                                                                 "Radius",
                                                                                                                 &component.radius,
                                                                                                                 0.1f,
                                                                                                                 0.0f,
                                                                                                                 100.0f);
                                                                                                     });
                                                                                         });
        ComponentUIRegistry::RegisterComponent<Components::BoundingSphereWorldComponent>("Bounding Sphere World",
                                                                                         [](Entity entity) {
                                                                                             UI::DrawComponent<Components::BoundingSphereWorldComponent>(
                                                                                                     "Bounding Sphere World",
                                                                                                     entity,
                                                                                                     [](auto &component) {
                                                                                                         ImGui::Text(
                                                                                                                 "Center: %s",
                                                                                                                 glm::to_string(
                                                                                                                         component.center).c_str());
                                                                                                         ImGui::DragFloat(
                                                                                                                 "Radius",
                                                                                                                 &component.radius,
                                                                                                                 0.1f,
                                                                                                                 0.0f,
                                                                                                                 100.0f);
                                                                                                     });
                                                                                         });
        ComponentUIRegistry::RegisterComponent<Components::CameraComponent>("Camera Cache", [&](Entity entity) {
            UI::DrawComponent<Components::CameraComponent>("Camera Cache", entity, [&](auto &component) {
                ImGui::Text("Projection Matrix: %s", glm::to_string(component.projection_matrix).c_str());
                ImGui::Text("View Matrix: %s", glm::to_string(component.view_matrix).c_str());

                ImGui::Text("Projection Type: %s", std::visit([](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, Components::CameraComponent::Perspective>) {
                        return "Perspective";
                    } else if constexpr (std::is_same_v<T, Components::CameraComponent::Orthographic>) {
                        return "Orthographic";
                    } else {
                        return "Unknown";
                    }
                }, component.projection));

                if (std::holds_alternative<Components::CameraComponent::Perspective>(component.projection)) {
                    auto &perspective = std::get<Components::CameraComponent::Perspective>(component.projection);
                    ImGui::DragFloat("FOV", &perspective.fov, 0.1f, 1.0f, 180.0f);
                    ImGui::DragFloat("Aspect Ratio", &perspective.aspect_ratio, 0.01f, 0.1f, 10.0f);
                    if (ImGui::Button("Make Orthographic")) {
                        // Convert to orthographic projection
                        component.projection = Components::CameraComponent::Orthographic{
                                -1.0f, 1.0f, -1.0f, 1.0f // Default values, can be adjusted
                        };
                    }
                } else if (std::holds_alternative<Components::CameraComponent::Orthographic>(component.projection)) {
                    auto &orthographic = std::get<Components::CameraComponent::Orthographic>(component.projection);
                    ImGui::DragFloat("Left", &orthographic.left, 0.1f, -100.0f, 100.0f);
                    ImGui::DragFloat("Right", &orthographic.right, 0.1f, -100.0f, 100.0f);
                    ImGui::DragFloat("Bottom", &orthographic.bottom, 0.1f, -100.0f, 100.0f);
                    ImGui::DragFloat("Top", &orthographic.top, 0.1f, -100.0f, 100.0f);
                    if (ImGui::Button("Make Perspective")) {
                        // Convert to perspective projection
                        component.projection = Components::CameraComponent::Perspective{
                                45.0f, 1.0f // Default values, can be adjusted
                        };
                    }
                    // Convert to orthographic projection
                    component.projection = Components::CameraComponent::Orthographic{
                            -1.0f, 1.0f, -1.0f, 1.0f // Default values, can be adjusted
                    };
                }

                ImGui::DragFloat("Z Near", &component.z_near, 0.1f, 0.01f, 100.0f);
                ImGui::DragFloat("Z Far", &component.z_far, 0.1f, 0.01f, 1000.0f);

                bool is_primary = m_scene->get_registry().all_of<Components::IsPrimaryTag<Components::CameraComponent>>(
                        entity);
                if (ImGui::Checkbox("Is Primary", &is_primary)) {
                    if (is_primary) {
                        m_scene->get_registry().emplace<Components::IsPrimaryTag<Components::CameraComponent>>(entity);
                    } else {
                        m_scene->get_registry().remove<Components::IsPrimaryTag<Components::CameraComponent>>(entity);
                    }
                }
            });
        });


        ComponentUIRegistry::RegisterComponent<Components::ColliderComponent>("Collider", [](Entity entity) {
            UI::DrawComponent<Components::ColliderComponent>("Collider", entity, [](auto &component) {

                ImGui::Text("Type: %s",
                            component.type == Components::ColliderComponent::ShapeType::Box ? "Box" :
                            component.type == Components::ColliderComponent::ShapeType::Sphere ? "Sphere" :
                            component.type == Components::ColliderComponent::ShapeType::Capsule ? "Capsule" :
                            component.type == Components::ColliderComponent::ShapeType::ConvexMesh ? "Convex Mesh" :
                            component.type == Components::ColliderComponent::ShapeType::TriangleMesh ? "Triangle Mesh" :
                            "Unknown");

                ImGui::DragFloat3("Offset", glm::value_ptr(component.offset), 0.1f, -100.0f, 100.0f);

                // Later: Add a field for the physics asset.
            });
        });

        ComponentUIRegistry::RegisterComponent<Components::MaterialComponent>("Material", [](Entity entity) {
            UI::DrawComponent<Components::MaterialComponent>("Material", entity, [](auto &component) {
                // Later: Display material properties
            });
        });
        ComponentUIRegistry::RegisterComponent<Components::RenderableComponent>("Renderable", [](Entity entity) {
            UI::DrawComponent<Components::NameTagComponent>("Renderable", entity, [](auto &component) {
                // TODO display renderable properties
            });
        });

        ComponentUIRegistry::RegisterComponent<Components::RigidBodyComponent>("Rigid Body", [](Entity entity) {
            UI::DrawComponent<Components::RigidBodyComponent>("Rigid Body", entity, [](auto &component) {
                //TODO choose body type: Static, Kinematic, Dynamic
                ImGui::DragFloat3("Velocity", glm::value_ptr(component.velocity), 0.1f, -100.0f, 100.0f);
                ImGui::DragFloat3("Angular Velocity", glm::value_ptr(component.angular_velocity), 0.1f, -100.0f,
                                  100.0f);
                ImGui::DragFloat("Mass", &component.mass, 0.1f, 0.1f, 1000.0f);
                ImGui::Checkbox("Disable Gravity", &component.disable_gravity);
            });
        });

        ComponentUIRegistry::RegisterComponent<Components::SkeletonComponent>("Skeleton", [](Entity entity) {
            UI::DrawComponent<Components::SkeletonComponent>("Skeleton", entity, [](auto &component) {
                //TODO display skeleton properties when implemented
            });
        });

        ComponentUIRegistry::RegisterComponent<Components::Transform>("Transform", [](Entity entity) {
            UI::DrawComponent<Components::Transform>("Transform", entity, [](auto &component) {
                ImGui::DragFloat3("Translation", glm::value_ptr(component.position), 0.1f);
                glm::vec3 euler_angles = glm::eulerAngles(component.rotation);
                ImGui::DragFloat3("Rotation (Euler)", glm::value_ptr(euler_angles), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(component.scale), 0.1f);
            });
        });
    }

    void EditorLayer::save_scene(const std::string &filepath) {

    }

    void EditorLayer::create_renderable_entity_from_asset(const std::string &file_path) {
        Application &app = Application::get();
        auto &asset_manager = app.get_asset_manager();
        auto asset_handle = asset_manager.load(file_path);
        switch (asset_handle.get_type()) {
            case AssetType::Geometry: {
                // Create a new entity with the loaded geometry asset.
                Entity entity = m_scene->create_entity("Renderable Entity");
                auto &renderable_component = entity.add_component<Components::RenderableComponent>();
                renderable_component.geometry_handle = asset_handle;
                renderable_component.material_handle = asset_manager.load("assets/materials/default_material.rde");
                m_selected_entity = entity; // Select the newly created entity
                break;
            }
            case AssetType::Texture:
            case AssetType::Material:
            case AssetType::Shader:
            case AssetType::Scene:
            default:
                RDE_CORE_ERROR("Unsupported asset type for creating renderable entity");
                break;
        }
    }
}
