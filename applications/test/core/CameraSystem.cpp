#include "CameraSystem.h"
#include "Camera.h"

#include "TransformSystem.h"
#include "Transform.h"

namespace RDE::CameraSystem {
    void set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
        registry.emplace_or_replace<Camera::Dirty>(entity_id);
    }

    void init(entt::registry &registry) {
        registry.on_construct<Camera::ViewParameters>().connect<&set_dirty_on_change>();
        registry.on_update<Camera::ViewParameters>().connect<&set_dirty_on_change>();

        registry.on_construct<Camera::ProjectionParameters>().connect<&set_dirty_on_change>();
        registry.on_update<Camera::ProjectionParameters>().connect<&set_dirty_on_change>();

        auto default_camera_entity = create_camera_entity(registry);
        make_camera_entity_primary(registry, default_camera_entity);
    }

    void shutdown(entt::registry &registry) {
        // Cleanup the camera system, if needed
        // This could include removing cameras or other cleanup tasks
        registry.clear<Camera::ViewParameters>();
        registry.clear<Camera::ProjectionParameters>();
        registry.clear<Camera::Matrices>();
        registry.clear<Camera::Dirty>();
        registry.clear<Camera::Primary>();
    }

    void update_dirty_cameras(entt::registry& registry) {
        // Query for all dirty cameras that have the necessary components
        auto view = registry.view<Camera::ProjectionParameters, Camera::Dirty>();

        for (auto entity : view) {
            const auto& proj_params = view.get<Camera::ProjectionParameters>(entity);
            glm::mat4 view_matrix;

            // Check if the camera is attached to the scene graph
            if (auto* transform = registry.try_get<Transform::Component>(entity)) {
                // If yes, derive the view matrix from the world-space transform.
                // The view matrix is the inverse of the camera's world matrix.
                view_matrix = glm::inverse(transform->world_matrix);
            } else if (auto* view_params = registry.try_get<Camera::ViewParameters>(entity)) {
                // If no, fall back to the static parameters (your current behavior).
                view_matrix = CalculateViewMatrix(*view_params);
            } else {
                // Not a valid camera, skip.
                continue;
            }

            // Calculate projection matrix as before
            glm::mat4 proj_matrix = CalculateProjectionMatrix(proj_params);

            // Store the final derived matrices
            registry.emplace_or_replace<Camera::Matrices>(entity, view_matrix, proj_matrix);
        }

        registry.clear<Camera::Dirty>();
    }

    entt::entity create_camera_entity(entt::registry &registry, entt::entity entity_id) {
        if (entity_id == entt::null) {
            entity_id = registry.create();
        }
        if (!registry.valid(entity_id)) return entt::null;

        if (!registry.all_of<Transform::Component>(entity_id)) {
            registry.emplace<Transform::Component>(entity_id);
        }
        if (!registry.all_of<Camera::ProjectionParameters>(entity_id)) {
            registry.emplace<Camera::ProjectionParameters>(entity_id);
        }

        registry.emplace_or_replace<Camera::Dirty>(entity_id);
        return entity_id;
    }

    bool make_camera_entity_primary(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<Transform::Component, Camera::ProjectionParameters>(entity_id)) {
            return false; // Invalid entity or not a camera
        }

        // Remove the primary camera tag from any existing primary camera
        registry.clear<Camera::Primary>();

        // Add the primary camera tag to the new camera entity
        registry.emplace<Camera::Primary>(entity_id);
        return true;
    }

    entt::entity get_primary_camera(entt::registry &registry) {
        auto view = registry.view<Camera::Primary>();
        if (view.empty()) {
            return entt::null; // No primary camera found
        }
        // Return the first entity with the PrimaryCamera tag
        return *view.begin();
    }

    void set_camera_dirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<Camera::ViewParameters, Camera::ProjectionParameters>(entity_id)) {
            return; // Invalid entity or not a camera
        }

        set_dirty_on_change(registry, entity_id);
    }
}
