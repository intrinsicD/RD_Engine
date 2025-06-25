#pragma once

#include "AssetID.h"

namespace RDE {
    class AssetHandle {
    public:
        explicit AssetHandle(AssetID id = INVALID_ASSET_ID, AssetType type = AssetType::None) : m_asset_id(id),
            m_type(type) {
        }

        AssetID get_asset_id() const {
            return m_asset_id;
        }

        bool is_valid() const {
            return m_asset_id != INVALID_ASSET_ID;
        }

        AssetType get_type() const {
            return m_type;
        }

        // Overloads for use as a key in maps/sets
        bool operator==(const AssetHandle &other) const {
            return m_asset_id == other.m_asset_id;
        }

        bool operator!=(const AssetHandle &other) const {
            return m_asset_id != other.m_asset_id;
        }

    private:
        AssetID m_asset_id;
        AssetType m_type;
    };
}
