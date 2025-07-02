#include "CameraSystem.h"
#include "Camera.h"

namespace RDE::CameraSystem {
    void init(entt::registry &registry) {
        // Initialize the camera system, if needed
        // This could include setting up default cameras or other initial state

        // For example, you might want to create a default camera entity
        auto default_camera_entity = create_camera_entity(registry);
        make_camera_entity_primary(registry, default_camera_entity);

        registry.on_construct<CameraViewParameters>().connect<&set_camera_dirty>();
        registry.on_construct<CameraProjectionParameters>().connect<&set_camera_dirty>();
        registry.on_update<CameraViewParameters>().connect<&set_camera_dirty>();
        registry.on_update<CameraProjectionParameters>().connect<&set_camera_dirty>();
    }

    void shutdown(entt::registry &registry) {
        // Cleanup the camera system, if needed
        // This could include removing cameras or other cleanup tasks
        registry.clear<CameraViewParameters>();
        registry.clear<CameraProjectionParameters>();
        registry.clear<CameraMatrices>();
        registry.clear<DirtyCamera>();
        registry.clear<PrimaryCamera>();
    }

    void update_dirty_cameras(entt::registry &registry) {
        auto view = registry.view<CameraViewParameters, CameraProjectionParameters, DirtyCamera>();
        for (auto entity: view) {
            const auto &camera_view_params = registry.get<CameraViewParameters>(entity);
            const auto &camera_projection_params = registry.get<CameraProjectionParameters>(entity);

            CameraMatrices camera_matrices{
                    .view_matrix = CalculateViewMatrix(camera_view_params),
                    .projection_matrix = CalculateProjectionMatrix(camera_projection_params)
            };
            registry.emplace_or_replace<CameraMatrices>(entity, camera_matrices);
        }
        //clear the view DirtyCamera components all at once
        registry.clear<DirtyCamera>();
    }

    entt::entity create_camera_entity(entt::registry &registry, entt::entity entity_id) {
        if (entity_id == entt::null) {
            entity_id = registry.create();
        }
        if (!registry.valid(entity_id)) return entt::null;

        if (!registry.all_of<CameraViewParameters>(entity_id)) {
            registry.emplace<CameraViewParameters>(entity_id);
        }
        if (!registry.all_of<CameraProjectionParameters>(entity_id)) {
            registry.emplace<CameraProjectionParameters>(entity_id);
        }

        registry.emplace_or_replace<DirtyCamera>(entity_id);
        return entity_id;
    }

    bool make_camera_entity_primary(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<CameraViewParameters, CameraProjectionParameters>(entity_id)) {
            return false; // Invalid entity or not a camera
        }

        // Remove the primary camera tag from any existing primary camera
        registry.clear<PrimaryCamera>();

        // Add the primary camera tag to the new camera entity
        registry.emplace<PrimaryCamera>(entity_id);
        return true;
    }

    entt::entity get_primary_camera(entt::registry &registry) {
        auto view = registry.view<PrimaryCamera>();
        if (view.empty()) {
            return entt::null; // No primary camera found
        }
        // Return the first entity with the PrimaryCamera tag
        return *view.begin();
    }

    void set_camera_dirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<CameraViewParameters, CameraProjectionParameters>(entity_id)) {
            return; // Invalid entity or not a camera
        }

        // Add the DirtyCamera tag to mark the camera as dirty
        registry.emplace_or_replace<DirtyCamera>(entity_id);
    }
}