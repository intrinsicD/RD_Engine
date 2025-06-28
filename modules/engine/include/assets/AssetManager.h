#pragma once

#include "AssetHandle.h" // You must have this file (see below)
#include "IAssetLoader.h" // You must have this file (see below)

#include <entt/entt.hpp>

namespace RDE {
    class IGraphicsDevice;

    class AssetManager {
    public:
        explicit AssetManager(IGraphicsDevice *device);

        ~AssetManager() = default;

        void register_loader(std::unique_ptr<IAssetLoader> loader);

        AssetHandle load(const std::string &path);

        template<typename T>
        T *try_get_component(const AssetHandle &handle) {
            if (!handle.is_valid()) {
                return nullptr;
            }
            return m_registry.try_get<T>(handle.m_asset_id);
        }
    private:
        void release_asset(const std::string& uri, entt::entity* entity_ptr);
        IGraphicsDevice *m_graphics_device;

        entt::registry m_registry;
        std::unordered_map<std::string, entt::entity> m_uri_cache;
        std::unordered_map<std::string, IAssetLoader *> m_loaders;
        std::unordered_map<std::string, uint32_t> m_ref_counts;
        std::vector<std::unique_ptr<IAssetLoader> > m_registered_loaders; // Owns the loaders
    };
}
