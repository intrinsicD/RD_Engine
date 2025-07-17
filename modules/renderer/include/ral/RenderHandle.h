//ral/RenderHandle
#pragma once

#include <entt/entity/entity.hpp>

namespace RAL{
    struct RenderHandle {
        entt::entity index = entt::null;

        RenderHandle() = default;

        explicit RenderHandle(entt::entity entity) : index(entity) {}

        RenderHandle &operator=(const RenderHandle &other) {
            if (this == &other) return *this; // Self-assignment check
            index = other.index; // Copy the entity index
            return *this;
        }

        // Define an explicit invalid state.
        static constexpr RenderHandle INVALID() { return {}; }

        constexpr bool is_valid() const {
            return index != entt::null;
        }

        auto version() const {
            return entt::to_version(index);
        }

        // For use in std::map, etc.
        bool operator<(const RenderHandle &other) const {
            if (index < other.index) return true;
            if (index > other.index) return false;
            return version() < other.version();
        }

        bool operator==(const RenderHandle &other) const {
            return index == other.index && version() == other.version();
        }
    };
}