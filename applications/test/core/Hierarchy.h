#pragma once

#include <entt/entity/entity.hpp>

namespace RDE {
    struct Hierarchy {
        entt::entity parent = entt::null; // Entity ID of this node
        entt::entity first_child = entt::null; // First child entity
        entt::entity last_child = entt::null; // Last child entity
        entt::entity next_sibling = entt::null; // Next sibling entity
        entt::entity prev_sibling = entt::null; // Previous sibling entity
        size_t num_children = 0;
    };
}
