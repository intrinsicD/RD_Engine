#pragma once

#include "components/TransformComponent.h"

#include <imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace RDE {
    struct TransformInspectorPanelGui {
        TransformLocal *transform_local = nullptr;
        TransformWorld *transform_world = nullptr;

        operator bool() const {
            return transform_local || transform_world;
        }

        void Draw() {
            if (transform_local) {
                ImGui::Text("Local Transform");
                ImGui::DragFloat3("Translation", &transform_local->translation.x, 0.1f);
                ImGui::DragFloat4("Orientation (Quaternion)", &transform_local->orientation.x, 0.1f);
                ImGui::DragFloat3("Scale", &transform_local->scale.x, 0.1f);
            }
            if (transform_world) {
                if (transform_local) ImGui::Separator();
                ImGui::Text("World Transform");
                ImGui::Text("Matrix:");
                ImGui::Text("%s", glm::to_string(transform_world->matrix).c_str());
            }
        }
    };
}