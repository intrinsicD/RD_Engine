#pragma once

#include "CameraComponent.h"

#include <imgui.h>
#include <glm/gtx/string_cast.hpp> // For glm::to_string

namespace RDE{
    struct CameraInspectorPanelGui {
        // This struct can be used to hold references to camera components
        // and provide a GUI for inspecting and modifying them.

        // Example camera component references can be added here
        // CameraComponent* camera = nullptr;
        CameraMatrices *camera_matrices = nullptr;
        CameraViewParameters *camera_view_params = nullptr;
        CameraProjectionParameters *camera_projection_params = nullptr;
        CameraFrustumPlanes *camera_frustum_planes = nullptr;
        bool camera_dirty = false;
        bool camera_primary = false;

        operator bool() const {
            // Return true if any camera components are present
            // return camera != nullptr;
            return camera_matrices != nullptr ||
                   camera_view_params != nullptr ||
                   camera_projection_params != nullptr ||
                   camera_frustum_planes != nullptr ||
                   camera_dirty ||
                   camera_primary;
        }

        void Draw() {
            bool any = false;
            if(camera_matrices){
                // Draw camera matrices
                ImGui::Text("View Matrix: %s", glm::to_string(camera_matrices->view_matrix).c_str());
                ImGui::Text("Projection Matrix: %s", glm::to_string(camera_matrices->projection_matrix).c_str());
                any = true;
            }
            if(camera_view_params){
                // Draw camera view parameters
                if(any) ImGui::Separator();
                ImGui::Text("Camera Position: %s", glm::to_string(camera_view_params->position).c_str());
                ImGui::Text("Forward Vector: %s", glm::to_string(camera_view_params->forward).c_str());
                ImGui::Text("Up Vector: %s", glm::to_string(camera_view_params->up).c_str());
                ImGui::Text("Right Vector: %s", glm::to_string(camera_view_params->get_right()).c_str());
                any = true;
            }
            if(camera_projection_params){
                // Draw camera projection parameters
                if(any) ImGui::Separator();
                ImGui::Text("Projection Type: %s", std::visit([](auto&& arg) { return arg.name; }, camera_projection_params->parameters).c_str());
                ImGui::Text("Near Plane: %.2f", camera_projection_params->near_plane);
                ImGui::Text("Far Plane: %.2f", camera_projection_params->far_plane);
                std::visit([](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, CameraProjectionParameters::Perspective>) {
                        ImGui::Text("FOV (degrees): %.2f", arg.fov_degrees);
                        ImGui::Text("Aspect Ratio: %.2f", arg.aspect_ratio);
                    } else if constexpr (std::is_same_v<T, CameraProjectionParameters::Orthographic>) {
                        ImGui::Text("Left: %.2f, Right: %.2f, Bottom: %.2f, Top: %.2f", arg.left, arg.right, arg.bottom, arg.top);
                    }
                }, camera_projection_params->parameters);
                any = true;
            }
            if(camera_frustum_planes) {
                // Draw camera frustum planes
                if (any) ImGui::Separator();
                ImGui::Text("Frustum Planes:");
                for (int i = 0; i < 6; ++i) {
                    ImGui::Text("Plane %d: %s", i, glm::to_string(camera_frustum_planes->planes[i].normal).c_str());
                }
                any = true;
            }

            ImGui::Text("Camera Dirty: %s", camera_dirty ? "Yes" : "No");
            ImGui::Text("Camera Primary: %s", (camera_primary ? "Yes" : "No"));
        }
    };
}