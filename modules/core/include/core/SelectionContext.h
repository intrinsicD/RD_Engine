#pragma once

#include <vector>
#include <entt/entity/entity.hpp>

namespace RDE {
    struct SelectionContext {
        entt::entity last_selected_entity;
        std::vector<entt::entity> selected_entities;
    };
}