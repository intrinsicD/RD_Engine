#include "assets/AssetManager.h"
#include "assets/Asset.h"
#include "assets/IAssetLoader.h"
#include "Log.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    AssetManager::AssetManager(IGraphicsDevice *device);

    void AssetManager::register_loader(std::unique_ptr<IAssetLoader> loader) {
        for (const auto& ext : loader->get_supported_extension()) {
            m_loaders[ext] = loader.get();
        }
        m_registered_loaders.push_back(std::move(loader));
    }

    AssetHandle AssetManager::load(const std::string &uri) {
        // 1. Check the cache
        if (auto it = m_uri_cache.find(uri); it != m_uri_cache.end()) {
            // Found it, return a new shared_ptr handle to the existing entity
            entt::entity entity = it->second;
            RDE_CORE_INFO("AssetManager: Cache hit for (uri:{}, asset_handle_id:{})", uri, entity);

            return {entity, &m_registry};
        }

        RDE_CORE_INFO("AssetManager: Cache miss for: (uri:{}). Loading...", uri.c_str());

        // 2. Find the correct loader
        std::string extension = uri.substr(uri.find_last_of('.'));
        auto loader_it = m_loaders.find(extension);
        if (loader_it == m_loaders.end()) {
            throw std::runtime_error("No loader registered for extension: " + extension);
        }

        // 3. Create a new entity and load data into it
        entt::entity new_entity = m_registry.create();
        m_registry.emplace<AssetFilepath>(new_entity, uri); // Store its path

        loader_it->second->load(uri, m_registry, new_entity);

        // 4. Update the cache and reference count
        m_uri_cache[uri] = new_entity;
        m_ref_counts[uri] = 1;

        // 5. Return the first handle (with custom deleter)
        return{new_entity, &m_registry};
    }

    void AssetManager::release_asset(const std::string& uri, entt::entity* entity_ptr) {
        RDE_CORE_INFO("AssetManager: Releasing asset: (uri:{})", uri);
        m_ref_counts[uri]--;

        if (m_ref_counts[uri] == 0) {
            RDE_CORE_INFO("AssetManager: Unloading asset (uri:{})", uri);
            entt::entity entity_to_destroy = *entity_ptr;

            m_registry.destroy(entity_to_destroy);
            m_uri_cache.erase(uri);
            m_ref_counts.erase(uri);
        }
        delete entity_ptr;
    }
}