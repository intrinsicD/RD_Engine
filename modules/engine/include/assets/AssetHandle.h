#pragma once

#include <entt/entity/entity.hpp>
#include <memory>

namespace RDE {
    // Forward declare to break circular dependency
    class AssetManager;

    // The AssetID is a shared pointer to the asset's entity ID.
    // Its custom deleter is the magic that links its lifetime back to the AssetManager.
    // When the last shared_ptr is destroyed, the deleter is invoked automatically.
    using AssetID = std::shared_ptr<entt::entity>;

    // A strongly-typed wrapper around the untyped AssetID.
    // The AssetType is just a "tag" (like struct MeshTag;), it's not stored.
    template <typename AssetType>
    struct AssetHandle {
        AssetID internal_handle;

        operator bool() const { return internal_handle && *internal_handle != entt::null; }

        AssetHandle() = default;
    private:
        friend class AssetManager;
        explicit AssetHandle(AssetID handle) : internal_handle(std::move(handle)) {}
    };
}
