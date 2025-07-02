#pragma once

#include <entt/entity/registry.hpp>

namespace RDE::HierarchySystem {
    void init(entt::registry &registry);

    void shutdown(entt::registry &registry);

    void set_parent(entt::registry &registry, entt::entity child_entity, entt::entity parent_entity);

    void remove_parent(entt::registry &registry, entt::entity child_entity);

    void update_dirty_hierarchy(entt::registry &registry);
}