#include "BoundingVolumeSystem.h"
#include "BoundingVolume.h"
#include "Transform.h"
#include "SystemDependencyBuilder.h"

#include <entt/entity/registry.hpp>
#include <glm/gtx/component_wise.hpp>

namespace RDE {
    namespace Detail {
        inline void set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
            registry.emplace_or_replace<BoundingVolumeDirty>(entity_id);
        }
    }

    BoundingVolumeSystem::BoundingVolumeSystem(entt::registry &registry) : m_registry(registry) {

    }

    void BoundingVolumeSystem::init() {
        m_registry.on_construct<BoundingVolumeAABBComponent>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_update<BoundingVolumeAABBComponent>().connect<&Detail::set_dirty_on_change>();

        m_registry.on_construct<BoundingVolumeSphereComponent>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_update<BoundingVolumeSphereComponent>().connect<&Detail::set_dirty_on_change>();

        m_registry.on_construct<BoundingVolumeCapsuleComponent>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_update<BoundingVolumeCapsuleComponent>().connect<&Detail::set_dirty_on_change>();
    }

    void BoundingVolumeSystem::shutdown() {
        m_registry.clear<BoundingVolumeAABBComponent>();
        m_registry.clear<BoundingVolumeSphereComponent>();
        m_registry.clear<BoundingVolumeCapsuleComponent>();
        m_registry.clear<BoundingVolumeDirty>();
    }

    void BoundingVolumeSystem::update(float delta_time) {
        {
            auto group = m_registry.group<BoundingVolumeAABBComponent>(entt::get<BoundingVolumeDirty>);
            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolumeAABBComponent>(entity);
                const auto *world = m_registry.try_get<TransformWorld>(entity);

                if (!world) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = world->matrix;

                // Calculate the world AABB from the local AABB
                const std::array<glm::vec3, 8> corners = GetCorners(bounding_volume.local);
                auto world_min = glm::vec3(std::numeric_limits<float>::max());
                auto world_max = glm::vec3(std::numeric_limits<float>::lowest());

                for (const auto &corner: corners) {
                    glm::vec3 world_corner = model_matrix * glm::vec4(corner, 1.0f);
                    world_min = glm::min(world_min, world_corner);
                    world_max = glm::max(world_max, world_corner);
                }
                bounding_volume.world.min = world_min;
                bounding_volume.world.max = world_max;
            }
        }
        {
            auto group = m_registry.group<BoundingVolumeSphereComponent>(entt::get<BoundingVolumeDirty>);

            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolumeSphereComponent>(entity);
                const auto *local = m_registry.try_get<TransformLocal>(entity);
                const auto *world = m_registry.try_get<TransformWorld>(entity);

                if (!world) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = world->matrix;
                const auto &scale = local->scale;

                // Calculate the world sphere from the local sphere
                bounding_volume.world.center = model_matrix * glm::vec4(bounding_volume.local.center, 1.0f);
                bounding_volume.world.radius = bounding_volume.local.radius * glm::compMax(scale);
            }
        }
        {
            auto group = m_registry.group<BoundingVolumeCapsuleComponent>(entt::get<BoundingVolumeDirty>);

            for (auto entity: group) {
                auto &bounding_volume = group.get<BoundingVolumeCapsuleComponent>(entity);
                const auto *local = m_registry.try_get<TransformLocal>(entity);
                const auto *world = m_registry.try_get<TransformWorld>(entity);

                if (!world) {
                    // Handle case with no transform (just copy local to world)
                    bounding_volume.world = bounding_volume.local; // Assuming you add 'local'
                    continue;
                }

                const auto &model_matrix = world->matrix;
                const auto &scale = local->scale;

                // Calculate the world capsule from the local capsule
                bounding_volume.world.segment.start = model_matrix * glm::vec4(
                        bounding_volume.local.segment.start, 1.0f);
                bounding_volume.world.segment.end = model_matrix * glm::vec4(bounding_volume.local.segment.end, 1.0f);
                bounding_volume.world.radius = bounding_volume.local.radius * glm::compMax(scale);
            }
        }

        m_registry.clear<BoundingVolumeDirty>();
    }

    void BoundingVolumeSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        builder.reads<BoundingVolumeAABBComponent>();
        builder.reads<BoundingVolumeSphereComponent>();
        builder.reads<BoundingVolumeCapsuleComponent>();
        builder.reads<BoundingVolumeDirty>();
        builder.reads<TransformLocal>();
        builder.reads<TransformWorld>();

        builder.writes<BoundingVolumeDirty>();
        builder.writes<BoundingVolumeAABBComponent>();
        builder.writes<BoundingVolumeSphereComponent>();
        builder.writes<BoundingVolumeCapsuleComponent>();
    }
}
