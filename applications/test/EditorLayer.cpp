#include "EditorLayer.h"
#include "SandboxApp.h"
#include "scene/SceneInspectorPanelGui.h"
#include "core/Log.h"
#include "core/InputManager.h"
#include "core/IWindow.h"
#include "components/TransformComponent.h"
#include "components/CameraComponent.h"
#include "components/BoundingVolumeComponent.h"
#include "components/NameTagComponent.h" // core variant only
#include "core/events/Event.h"
#include "core/events/MouseEvent.h"
#include "geometry/IntersectionRayAABB.h"
#include "scene/DefaultComponentGui.h"

#include <imgui.h>
#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <optional>

namespace RDE {
    void EditorLayer::on_attach() {
        RegisterDefaultComponentGui();
    }

    void EditorLayer::on_event(Event &e) {
        // Left click selection when not captured by ImGui
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &ev) {
            if (!ev.is_left_button()) return false;
            entt::entity hit = entt::null;
            if (pick_at_cursor(hit)) {
                if (m_app) m_app->set_last_selected_entity(hit);
                return true;
            }
            return false;
        });
    }

    void EditorLayer::on_render_gui() {
        // Contribute extra menus to existing main menu bar (already begun by ImGuiLayer)
        if (ImGui::BeginMenu("Editor")) {
            if (ImGui::MenuItem("Create Empty")) {
                auto e = m_registry.create();
                m_registry.emplace<TransformLocal>(e);
                if (!m_registry.all_of<NameTagComponent>(e))
                    m_registry.emplace<NameTagComponent>(e, NameTagComponent{"Entity"});
                if (m_app) m_app->set_last_selected_entity(e);
            }
            bool hasSel = m_app && m_app->get_last_selected_entity() != entt::null &&
                          m_registry.valid(m_app->get_last_selected_entity());
            if (ImGui::MenuItem("Delete Selected", nullptr, false, hasSel)) {
                auto e = m_app->get_last_selected_entity();
                if (e != entt::null && m_registry.valid(e)) {
                    m_registry.destroy(e);
                    m_app->set_last_selected_entity(entt::null);
                }
            }
            if (ImGui::MenuItem("Focus Camera", nullptr, false, hasSel)) {
                auto e = m_app->get_last_selected_entity();
                if (e != entt::null && m_registry.valid(e)) {
                    // Compute center/radius
                    glm::vec3 center{0};
                    float radius = 1.0f;
                    if (m_registry.all_of<BoundingVolumeAABBComponent>(e)) {
                        const auto &bv = m_registry.get<BoundingVolumeAABBComponent>(e);
                        if (bv.world.is_valid()) {
                            center = bv.world.center();
                            radius = glm::length(bv.world.half_extent());
                            radius = radius > 0.01f ? radius : 1.0f;
                        }
                    }
                    // Move primary camera to view this object
                    entt::entity cam = CameraUtils::GetCameraEntityPrimary(m_registry);
                    if (cam != entt::null && m_registry.valid(cam) && m_registry.all_of<TransformLocal>(cam)) {
                        auto &tl = m_registry.get<TransformLocal>(cam);
                        glm::vec3 forward = glm::normalize(tl.orientation * glm::vec3(0, 0, -1));
                        if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, -1);
                        float dist = 2.5f * radius;
                        tl.translation = center - forward * dist;
                        CameraUtils::SetCameraDirty(m_registry, cam);
                    }
                }
            }
            if (ImGui::MenuItem("Deselect", nullptr, false, m_app && m_app->get_last_selected_entity() != entt::null)) {
                m_app->set_last_selected_entity(entt::null);
            }
            ImGui::EndMenu();
        }

        draw_entity_list();
        if (m_app) {
            entt::entity selected = m_app->get_last_selected_entity();
            if (selected != entt::null && m_registry.valid(selected)) {
                draw_entity_inspector(selected);
            }
        }
    }

    std::string EditorLayer::get_entity_label(entt::entity e, bool selected) const {
        std::string base;
        if (m_registry.valid(e)) {
            if (m_registry.all_of<NameTagComponent>(e)) {
                base = m_registry.get<NameTagComponent>(e).name;
            } else {
                base = std::string("Entity ") + std::to_string((uint32_t) entt::to_integral(e));
            }
        }
        if (selected) base = "* " + base;
        return base;
    }

    void EditorLayer::draw_entity_list() {
        ImGui::Begin("Entities");
        static char filterBuf[128] = {0};
        ImGui::InputTextWithHint("##entity_filter", "filter (name or id)", filterBuf, sizeof(filterBuf));
        std::string filter = filterBuf;
        auto view = m_registry.view<entt::entity>(); // iteration over alive entities
        int count = 0;
        for (auto entity: view) {
            ++count;
            std::string label = get_entity_label(entity, m_app && entity == m_app->get_last_selected_entity());
            if (!filter.empty()) {
                auto idStr = std::to_string((uint32_t) entt::to_integral(entity));
                if (label.find(filter) == std::string::npos && idStr.find(filter) == std::string::npos) continue;
            }
            ImGui::PushID((int) entt::to_integral(entity));
            bool isSelected = (m_app && entity == m_app->get_last_selected_entity());
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                if (m_app) m_app->set_last_selected_entity(entity);
            }
            ImGui::PopID();
        }
        ImGui::TextDisabled("%d entities", count);
        ImGui::End();
    }

    void EditorLayer::draw_entity_inspector(entt::entity e) {
        ImGui::Begin("Inspector");
        ImGui::Text("Entity: %u", (uint32_t) entt::to_integral(e));
        ImGui::Separator();
        EntityInspectorPanelGui inspector{
                .entity = e,
                .registry = &m_registry
        };

        inspector.Draw();
        ImGui::End();
    }

    bool EditorLayer::pick_at_cursor(entt::entity &out_entity) const {
        out_entity = entt::null;
        // Get cursor and window size
        glm::vec2 cursor = InputManager::get_cursor_info().current_position;
        if (!m_app) return false;
        IWindow *win = m_app->get_window();
        if (!win) return false;
        int w = 0, h = 0;
        win->get_framebuffer_size(w, h);
        if (w <= 0 || h <= 0) return false;
        // Get primary camera and matrices
        entt::entity cam = CameraUtils::GetCameraEntityPrimary(m_registry);
        if (cam == entt::null || !m_registry.valid(cam)) return false;
        if (!m_registry.all_of<CameraMatrices>(cam)) return false;
        const auto &cm = m_registry.get<CameraMatrices>(cam);
        glm::vec3 camPos{0};
        if (m_registry.all_of<TransformWorld>(cam)) camPos = glm::vec3(m_registry.get<TransformWorld>(cam).matrix[3]);
        else if (m_registry.all_of<TransformLocal>(cam)) camPos = m_registry.get<TransformLocal>(cam).translation;

        // Build a ray from screen coords
        float x = (2.0f * cursor.x) / (float) w - 1.0f;
        float y = 1.0f - (2.0f * cursor.y) / (float) h;
        glm::vec4 ray_clip(x, y, -1.0f, 1.0f);
        glm::mat4 inv_proj = glm::inverse(cm.projection_matrix);
        glm::vec4 ray_eye = inv_proj * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
        glm::mat4 inv_view = glm::inverse(cm.view_matrix);
        glm::vec3 ray_dir = glm::normalize(glm::vec3(inv_view * ray_eye));

        // Traverse AABBs and pick nearest
        float best_t = std::numeric_limits<float>::infinity();
        entt::entity best = entt::null;
        auto view = m_registry.view<BoundingVolumeAABBComponent>();
        for (auto e: view) {
            const auto &bv = view.get<BoundingVolumeAABBComponent>(e);
            if (!bv.world.is_valid()) continue;
            auto result = Intersect({camPos, ray_dir}, bv.world);
            if (result.hit) {
                if (result.tmin < best_t) {
                    best_t = result.tmin;
                    best = e;
                }
            }
        }
        if (best != entt::null) {
            out_entity = best;
            return true;
        }
        return false;
    }
}
