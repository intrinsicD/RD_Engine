#pragma once

#include <entt/entity/entity.hpp>
#include <memory>

namespace RDE {
    // Forward declare to break circular dependency
    class AssetManager;

    // This struct IS the payload for our handle.
    struct AssetID_Data {
        entt::entity entity_id;
        std::string uri;
        // We can add more data here later without changing any function signatures!
    };

    using AssetID = std::shared_ptr<AssetID_Data>;

    template <typename AssetType>
    struct AssetHandle {
        AssetID internal_handle;

        operator bool() const { return internal_handle && internal_handle->entity_id != entt::null; }

        AssetHandle() = default;
    private:
        friend class AssetManager;
        explicit AssetHandle(AssetID handle) : internal_handle(std::move(handle)) {}
    };
}
