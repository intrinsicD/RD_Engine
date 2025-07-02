#pragma once

#include <entt/entity/registry.hpp>

namespace RDE::TransformSystem{
    void init(entt::registry &registry);

    void shutdown(entt::registry &registry);

    void update_dirty_transforms(entt::registry &registry);

    void set_transform_dirty(entt::registry &registry, entt::entity entity_id);
}