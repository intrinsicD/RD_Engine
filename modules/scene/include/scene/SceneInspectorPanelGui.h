#pragma once

#include "EntityInspectorPanelGui.h"

#include <entt/entity/registry.hpp>

namespace RDE{
    struct SceneInspectorPanelGui {
        entt::registry* registry; // The registry containing the scene entities

        void Draw() {
            if(!registry) {
                ImGui::Text("No registry available.");
                return;
            }
            auto view = registry->view<entt::entity>();
            for(auto entity : view) {
                auto integral_id = entt::to_integral(entity);
                ImGui::PushID((uint32_t)entt::to_integral(entity));
                ImGui::Text("Entity ID: %d", integral_id);

                // Create an EntityInspectorPanelGui for each entity
                EntityInspectorPanelGui inspector{
                    .entity = entity,
                    .registry = registry
                };

                inspector.Draw();

                ImGui::PopID();
            }
        }
    };
}