#include "BoundingVolumeSystem.h"
#include "BoundingVolume.h"
#include "Transform.h"

#include <glm/gtx/component_wise.hpp>

namespace RDE::BoundingVolumeSystem {
    void set_dirty_on_change(entt::registry &registry, const entt::entity entity_id) {
        registry.emplace_or_replace<BoundingVolume::Dirty>(entity_id);
    }

    void init(entt::registry &registry) {
        registry.on_construct<BoundingVolume::AABBComponent>().connect<&set_dirty_on_change>();
        registry.on_construct<BoundingVolume::SphereComponent>().connect<&set_dirty_on_change>();
        registry.on_construct<BoundingVolume::CapsuleComponent>().connect<&set_dirty_on_change>();
        registry.on_update<BoundingVolume::AABBComponent>().connect<&set_dirty_on_change>();
        registry.on_update<BoundingVolume::SphereComponent>().connect<&set_dirty_on_change>();
        registry.on_update<BoundingVolume::CapsuleComponent>().connect<&set_dirty_on_change>();
    }

    void shutdown(entt::registry &registry) {
        registry.clear<BoundingVolume::AABBComponent>();
        registry.clear<BoundingVolume::SphereComponent>();
        registry.clear<BoundingVolume::CapsuleComponent>();
        registry.clear<BoundingVolume::Dirty>();
    }

    void update_dirty_bounding_volumes(entt::registry &registry) { {
            auto group = registry.group<BoundingVolume::AABBComponent>(entt::get<BoundingVolume::Dirty>);
            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolume::AABBComponent>(entity);
                const auto *transform = registry.try_get<Transform::Component>(entity);

                if (!transform) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = transform->world_matrix;

                // Calculate the world AABB from the local AABB
                const std::array<glm::vec3, 8> corners = GetCorners(bounding_volume.local);
                glm::vec3 world_min = glm::vec3(std::numeric_limits<float>::max());
                glm::vec3 world_max = glm::vec3(std::numeric_limits<float>::lowest());

                for (const auto &corner: corners) {
                    glm::vec3 world_corner = model_matrix * glm::vec4(corner, 1.0f);
                    world_min = glm::min(world_min, world_corner);
                    world_max = glm::max(world_max, world_corner);
                }
                bounding_volume.world.min = world_min;
                bounding_volume.world.max = world_max;
            }
        } {
            auto group = registry.group<BoundingVolume::SphereComponent>(entt::get<BoundingVolume::Dirty>);

            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolume::SphereComponent>(entity);
                const auto *transform = registry.try_get<Transform::Component>(entity);

                if (!transform) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = transform->world_matrix;
                const auto &scale = transform->parameters.scale;

                // Calculate the world sphere from the local sphere
                bounding_volume.world.center = model_matrix * glm::vec4(bounding_volume.local.center, 1.0f);
                bounding_volume.world.radius = bounding_volume.local.radius * glm::compMax(scale);
            }
        } {
            auto group = registry.group<BoundingVolume::CapsuleComponent>(entt::get<BoundingVolume::Dirty>);

            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolume::CapsuleComponent>(entity);
                const auto *transform = registry.try_get<Transform::Component>(entity);

                if (!transform) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = transform->world_matrix;
                const auto &scale = transform->parameters.scale;

                // Calculate the world capsule from the local capsule
                bounding_volume.world.segment.start = model_matrix * glm::vec4(
                                                          bounding_volume.local.segment.start, 1.0f);
                bounding_volume.world.segment.end = model_matrix * glm::vec4(bounding_volume.local.segment.end, 1.0f);
                bounding_volume.world.radius = bounding_volume.local.radius * glm::compMax(scale);
            }
        }

        registry.clear<BoundingVolume::Dirty>();
    }

    void set_bounding_volume_dirty(entt::registry &registry, entt::entity entity) {
        if (!registry.valid(entity)) {
            return;
        }
        if (registry.all_of<BoundingVolume::AABBComponent>(entity) ||
            registry.all_of<BoundingVolume::SphereComponent>(entity) ||
            registry.all_of<BoundingVolume::CapsuleComponent>(entity)) {
            // If the entity has any bounding volume component, mark it as dirty
            set_dirty_on_change(registry, entity);
        }
    }
}
