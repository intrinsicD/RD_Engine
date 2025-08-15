#include "EditorLayer.h"
#include "SandboxApp.h"
#include "scene/SceneInspectorPanelGui.h"
#include "core/Log.h"

#include <imgui.h>
#include <entt/entity/registry.hpp>

namespace RDE {

    void EditorLayer::on_render_gui() {
        // Contribute extra menus to existing main menu bar (already begun by ImGuiLayer)
        if (ImGui::BeginMenu("Editor")) {
            if(ImGui::MenuItem("Deselect", nullptr, false, m_app && m_app->get_last_selected_entity()!=entt::null)) {
                m_app->set_last_selected_entity(entt::null);
            }
            ImGui::EndMenu();
        }

        draw_entity_list();
        if(m_app) {
            entt::entity selected = m_app->get_last_selected_entity();
            if(selected != entt::null && m_registry.valid(selected)) {
                draw_entity_inspector(selected);
            }
        }
    }

    inline std::string id_to_label(entt::entity e, bool selected) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%sEntity %u", selected?"* ":"", (uint32_t)entt::to_integral(e));
        return std::string(buf);
    }

    void EditorLayer::draw_entity_list() {
        ImGui::Begin("Entities");
        static char filterBuf[128] = {0};
        ImGui::InputTextWithHint("##entity_filter", "filter (id)", filterBuf, sizeof(filterBuf));
        std::string filter = filterBuf;
        auto view = m_registry.view<entt::entity>(); // iteration over all alive
        int count = 0;
        for(auto entity: view) {
            ++count;
            // simple filter by numeric id substring
            if(!filter.empty()) {
                auto idStr = std::to_string((uint32_t)entt::to_integral(entity));
                if(idStr.find(filter)==std::string::npos) continue;
            }
            ImGui::PushID((int)entt::to_integral(entity));
            bool isSelected = (m_app && entity == m_app->get_last_selected_entity());
            if(ImGui::Selectable(id_to_label(entity, isSelected).c_str(), isSelected)) {
                if(m_app) m_app->set_last_selected_entity(entity);
            }
            ImGui::PopID();
        }
        ImGui::TextDisabled("%d entities", count);
        ImGui::End();
    }



    void EditorLayer::draw_entity_inspector(entt::entity e) {
        ImGui::Begin("Inspector");
        ImGui::Text("Entity: %u", (uint32_t)entt::to_integral(e));
        ImGui::Separator();
        EntityInspectorPanelGui inspector{
                .entity = e,
                .registry = &m_registry
        };

        inspector.Draw();
        ImGui::End();
    }
}

