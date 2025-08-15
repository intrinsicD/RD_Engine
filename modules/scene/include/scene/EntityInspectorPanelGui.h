#pragma once

#include "components/TransformInspectorPanelGui.h"
#include "components/CameraInspectorPanelGui.h"

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

            // Retrieve components from the registry
            TransformInspectorPanelGui transformInspector{
                .transform_local = registry->try_get<TransformLocal>(entity),
                .transform_world = registry->try_get<TransformWorld>(entity)
            };

            if(!transformInspector) {
                ImGui::Text("No Transform components found for this entity.");
                return;
            }else{
                transformInspector.Draw();
            }

            CameraInspectorPanelGui cameraInspector{
                .camera_matrices = registry->try_get<CameraMatrices>(entity),
                .camera_view_params = registry->try_get<CameraViewParameters>(entity),
                .camera_projection_params = registry->try_get<CameraProjectionParameters>(entity),
                .camera_frustum_planes = registry->try_get<CameraFrustumPlanes>(entity),
                .camera_dirty = registry->all_of<CameraDirty>(entity),
                .camera_primary = registry->all_of<CameraPrimary>(entity)
            };

            if(!cameraInspector) {
                ImGui::Text("No Camera components found for this entity.");
            } else {
                cameraInspector.Draw();
            }
        }
    };
}