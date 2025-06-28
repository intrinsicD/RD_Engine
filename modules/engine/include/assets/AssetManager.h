#pragma once

#include "AssetHandle.h" // You must have this file (see below)

#include <entt/entt.hpp>

namespace RDE {
    class IGraphicsDevice;

    class AssetManager {
    public:
        explicit AssetManager(IGraphicsDevice *device);

        ~AssetManager() = default;

        AssetHandle load(const std::string &path);

        template<typename T>
        T *try_get_component(const AssetHandle &handle) {
            if (!handle.is_valid()) {
                return nullptr;
            }
            return m_registry.try_get<T>(handle.m_asset_id);
        }

        void destroy(const AssetHandle &handle);
    private:
        IGraphicsDevice *m_graphics_device;

        std::unordered_map<std::string, AssetHandle> m_asset_cache;
        entt::registry m_registry;
    };
}
