#pragma once

#include <cstdint>
#include <functional>

namespace RDE {
    using AssetID = uint32_t;
    const AssetID INVALID_ASSET_ID = 0;

    class AssetHandle {
    public:
        AssetHandle(AssetID id =  INVALID_ASSET_ID) : m_asset_id(id) {}

        AssetID get_asset_id() const {
            return m_asset_id;
        }

        bool is_valid() const {
            return m_asset_id != INVALID_ASSET_ID;
        }

        bool operator==(const AssetHandle &other) const {
            return m_asset_id == other.m_asset_id;
        }

        bool operator!=(const AssetHandle &other) const {
            return m_asset_id != other.m_asset_id;
        }



    private:
        AssetID m_asset_id = 0;
    };
}

namespace std {
    template<>
    struct hash<RDE::AssetHandle> {
        size_t operator()(const RDE::AssetHandle& handle) const {
            return hash<uint32_t>()(handle.get_asset_id());
        }
    };
}