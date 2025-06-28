#include "assets/AssetManager.h"
#include "assets/AssetComponentTypes.h"

namespace RDE {
    AssetManager::AssetManager() {

    }

    void AssetManager::register_loader(std::unique_ptr<IAssetLoader> loader) {
        for (const auto& ext : loader->get_supported_extensions()) {
            m_loaders[ext] = loader.get();
        }
        m_registered_loaders.push_back(std::move(loader));
    }

    void AssetManager::release_asset(const std::string& uri, entt::entity entity_to_destroy) {
        RDE_CORE_INFO("AssetManager: All handles released. Unloading asset (uri:{})", uri);

        // The registry's internal data for this entity is destroyed.
        m_registry.destroy(entity_to_destroy);

        // The asset is removed from the cache so it can be reloaded from disk next time.
        m_uri_cache.erase(uri);
    }
}