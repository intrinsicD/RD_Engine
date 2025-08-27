#pragma once

#include "components/TransformInspectorPanelGui.h"
#include "components/CameraInspectorPanelGui.h"
#include "scene/ComponentRegistry.h"

#include <entt/entity/registry.hpp>

namespace RDE{
    struct EntityInspectorPanelGui {
        // This struct can be used to hold references to various components of an entity
        // and provide a GUI for inspecting and modifying them.

        entt::entity entity; // The entity being inspected
        entt::registry* registry; // The registry containing the entity

        void Draw() {
            if(!registry || !registry->valid(entity)) {
                ImGui::Text("No valid entity selected.");
                return;
            }

            // Header row: Add/Remove helpers
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }
            ComponentRegistryGui::instance().draw_add_component_popup(*registry, entity);

            ImGui::Separator();

            // Iterate over all registered components and draw those that exist on this entity
            for (const auto &desc : ComponentRegistryGui::instance().descriptors()) {
                if (!desc.has(*registry, entity)) continue;
                ImGui::PushID((int)desc.type_hash);
                bool open = ImGui::TreeNode(desc.name.c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("Remove")) {
                    desc.remove(*registry, entity);
                    ImGui::PopID();
                    continue;
                }
                if (open) {
                    desc.draw(*registry, entity);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
    };
}