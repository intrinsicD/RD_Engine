#pragma once

#include <entt/entity/registry.hpp>

namespace RDE {
    template<typename Component>
    void Require(entt::registry &registry, entt::entity entity) {
        if (!registry.all_of<Component>(entity)) {
            registry.emplace<Component>(entity);
        }
    }

    template<typename Component>
    void Require(entt::registry &registry, entt::entity entity, Component &&component) {
        if (!registry.all_of<Component>(entity)) {
            registry.emplace<Component>(entity, std::forward<Component>(component));
        }
    }

    template<typename Component>
void Require(entt::registry &registry, entt::entity entity, const Component &component) {
        if (!registry.all_of<Component>(entity)) {
            registry.emplace<Component>(entity, component);
        }
    }

    void EnsureDefaultComponents(entt::registry &registry, entt::entity entity);
}
