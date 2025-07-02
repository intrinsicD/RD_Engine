#include "TransformSystem.h"
#include "Transform.h"
#include "BoundingVolumeSystem.h"
#include "CameraSystem.h"
#include "Camera.h"

namespace RDE::TransformSystem {
    void set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
        registry.emplace_or_replace<Transform::Dirty>(entity_id);
    }

    void init(entt::registry &registry) {
        // Initialize the Transform system by ensuring the necessary components are present
        registry.on_construct<Transform::Component>().connect<&set_dirty_on_change>();
        registry.on_update<Transform::Component>().connect<&set_dirty_on_change>();
    }

    void shutdown(entt::registry &registry) {
        // Cleanup if necessary, currently nothing to do
        registry.clear<Transform::Component>();
        registry.clear<Transform::Dirty>();
    }

    void update_dirty_transforms(entt::registry &registry) {
        auto view = registry.view<Transform::Component, Transform::Dirty>();
        for (auto entity: view) {
            auto &transform = view.get<Transform::Component>(entity);
            transform.world_matrix = get_model_matrix(transform.parameters); // Update the world matrix

            BoundingVolumeSystem::set_bounding_volume_dirty(registry, entity);
            CameraSystem::set_camera_dirty(registry, entity);
        }
        registry.clear<Transform::Dirty>();
    }

    void set_transform_dirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) || !registry.all_of<Transform::Component>(entity_id)) {
            return;
        }
        set_dirty_on_change(registry, entity_id);
    }
}