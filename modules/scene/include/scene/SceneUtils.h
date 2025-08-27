#pragma once

#include <entt/entity/registry.hpp>

namespace RDE{
    template<typename Component>
    void Requires(entt::registry &registry, entt::entity entity){
        if(!registry.all_of<Component>(entity)){
            registry.emplace<Component>(entity);
        }
    }

    void EnsureDefaultComponents(entt::registry &registry, entt::entity entity);
}