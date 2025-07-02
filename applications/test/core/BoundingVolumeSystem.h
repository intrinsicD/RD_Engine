#pragma once

#include <entt/entity/registry.hpp>

namespace RDE::BoundingVolumeSystem {
    void init(entt::registry &registry);

    void shutdown(entt::registry &registry);

    void update_dirty_bounding_volumes(entt::registry &registry);

    void set_bounding_volume_dirty(entt::registry &registry, entt::entity entity);
}