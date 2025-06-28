#pragma once

#include "AssetHandle.h" // You must have this file (see below)
#include "IAssetLoader.h" // You must have this file (see below)
#include "AssetComponentTypes.h" // You must have this file (see below)
#include "Log.h" // You must have this file (see below)

#include <entt/entity/registry.hpp>

namespace RDE {
    class AssetManager {
    public:
        explicit AssetManager();

        ~AssetManager() = default;

        void register_loader(std::unique_ptr<IAssetLoader> loader);

        // The load function is now a template that returns a strongly-typed handle.
        template<typename AssetType>
        AssetHandle<AssetType> load(const std::string &uri) {

            // 1. Find loader by extension
            std::string extension = uri.substr(uri.find_last_of('.'));
            auto loader_it = m_loaders.find(extension);
            if (loader_it == m_loaders.end()) {
                throw std::runtime_error("No loader for extension: " + extension);
            }
            IAssetLoader *loader = loader_it->second;

            // 2. CRITICAL: Verify the found loader produces the requested asset type.
            if (loader->get_asset_type() != std::type_index(typeid(AssetType))) {
                RDE_CORE_WARN("AssetManager: Loader for '{}' does not produce requested type '{}'.", uri,
                              typeid(AssetType).name());
                return AssetHandle<AssetType>(); // Return invalid handle
            }

            // 3. Check cache
            if (auto it = m_uri_cache.find(uri); it != m_uri_cache.end()) {
                // Attempt to lock the weak_ptr to get a shared_ptr (AssetID).
                // If it succeeds, the asset is still in memory.
                if (AssetID cached_id = it->second.lock()) {
                    // ... Perform type check always for robustness ...
                    if (loader->verify(m_registry, *cached_id)) {
                        RDE_CORE_INFO("AssetManager: Cache hit for '{}'", uri);
                        return AssetHandle<AssetType>(cached_id);
                    }
                }
            }

            // 4. Load the asset using the untyped interface
            entt::entity new_entity = loader->load(uri, m_registry);
            m_registry.emplace<AssetType>(new_entity);
            m_registry.emplace<AssetFilepath>(new_entity, uri);

            // 4. Create the shared_ptr (AssetID) with the custom deleter...
            AssetID asset_id(
                new entt::entity(new_entity),
                [this, uri, entity=new_entity](const entt::entity *e) {
                    this->release_asset(uri, entity);
                    delete e;
                }
            );

            // 5. Store a weak_ptr in the cache and return the typed handle.
            m_uri_cache[uri] = asset_id;
            return AssetHandle<AssetType>(asset_id);
        }

        template<typename AssetComponentType, typename AssetType>
        AssetComponentType &get(AssetHandle<AssetType> &handle) {
            // The manager uses the handle's ID to access its own private registry.
            // This is safe and encapsulated.
            if (!handle) throw std::runtime_error("Attempted to use an invalid asset handle.");
            return m_registry.get<AssetComponentType>(*handle);
        }

        // try_get version for safe access
        template<typename AssetComponentType, typename AssetType>
        AssetComponentType *try_get(AssetHandle<AssetType> &handle) {
            if (!handle || !m_registry.valid(*handle)) return nullptr;
            return m_registry.try_get<AssetComponentType>(*handle);
        }

    private:
        void release_asset(const std::string &uri, entt::entity entity_to_destroy);

        entt::registry m_registry;
        std::unordered_map<std::string, std::weak_ptr<entt::entity> > m_uri_cache;
        std::unordered_map<std::string, IAssetLoader *> m_loaders;
        std::vector<std::unique_ptr<IAssetLoader> > m_registered_loaders; // Owns the loaders
    };
}
