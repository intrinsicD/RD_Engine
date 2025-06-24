#pragma once

#include "AssetID.h"

namespace RDE{
    class AssetHandle {
    public:
        explicit AssetHandle(AssetID id = INVALID_ASSET_ID) : m_asset_id(id) {}

        AssetID get_asset_id() const {
            return m_asset_id;
        }

        bool is_valid() const {
            return m_asset_id != INVALID_ASSET_ID;
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
    };
}