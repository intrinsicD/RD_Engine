#pragma once

#include <entt/fwd.hpp>

namespace RDE {
    class AssetManager;

    class AssetHandle {
    public:
        AssetHandle();

        [[nodiscard]] bool is_valid() const ;

        [[nodiscard]] operator bool() const {
            return is_valid();
        }

        // Overloads for use as a key in maps/sets
        bool operator==(const AssetHandle &other) const;

        bool operator!=(const AssetHandle &other) const;

        // Define a hash function so it can be used in maps
        struct Hasher {
            size_t operator()(const AssetHandle& handle) const;
        };
    private:
        friend class AssetManager;

        AssetHandle(entt::entity asset_id, entt::registry *registry);

        entt::registry *m_registry;
        entt::entity m_asset_id; //TODO think if we need to store the registry here or not and if this should be a shared_ptr or not!?!?
    };
}
