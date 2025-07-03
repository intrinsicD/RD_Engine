#include "BoundingVolumeUtils.h"

#include <entt/entity/registry.hpp>

namespace RDE::BoundingVolumeUtils{
    void SetBoundingVolumeDirty(entt::registry &registry, entt::entity entity_id){
        if (!registry.valid(entity_id)) {
            return;
        }
        if (registry.all_of<BoundingVolumeAABBComponent>(entity_id) ||
            registry.all_of<BoundingVolumeSphereComponent>(entity_id) ||
            registry.all_of<BoundingVolumeCapsuleComponent>(entity_id)) {
            // If the entity has any bounding volume component, mark it as dirty
            registry.emplace_or_replace<BoundingVolumeDirty>(entity_id);
        }
    }
}