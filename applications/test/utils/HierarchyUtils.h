#pragma once

#include <entt/fwd.hpp>

namespace RDE::HierarchyUtils{
    void SetParent(entt::registry &registry, entt::entity child_entity, entt::entity parent_entity);

    void RemoveParent(entt::registry &registry, entt::entity child_entity);
}