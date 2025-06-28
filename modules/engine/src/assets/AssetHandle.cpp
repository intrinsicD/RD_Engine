#include "assets/AssetHandle.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    AssetHandle::AssetHandle() : m_asset_id(entt::null), m_registry(nullptr) {

    }

    bool AssetHandle::is_valid() const {
        return m_asset_id != entt::null && (m_registry ? m_registry->valid(m_asset_id) : true);
    }

    // Overloads for use as a key in maps/sets
    bool AssetHandle::operator==(const AssetHandle &other) const {
        return m_asset_id == other.m_asset_id;
    }

    bool AssetHandle::operator!=(const AssetHandle &other) const {
        return m_asset_id != other.m_asset_id;
    }

    AssetHandle::AssetHandle(entt::entity asset_id, entt::registry *registry) : m_asset_id(asset_id),
        m_registry(registry) {
    }

    size_t AssetHandle::Hasher::operator()(const AssetHandle& handle) const {
        return entt::to_integral(handle.m_asset_id);
    }
}
