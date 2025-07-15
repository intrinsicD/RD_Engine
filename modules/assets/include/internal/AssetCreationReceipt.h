//assets/internal/AssetCreationReceipt.h
#pragma once

#include <entt/fwd.hpp>

namespace RDE {
    struct AssetCreationReceipt {
        entt::entity entity_id;

        explicit AssetCreationReceipt(entt::entity id) : entity_id(id) {}

        AssetCreationReceipt() = default;
    };
}
