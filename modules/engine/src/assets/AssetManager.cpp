#include "assets/AssetManager.h"

namespace RDE {
    AssetManager::AssetManager(IGraphicsDevice *device);

    AssetHandle AssetManager::load(const std::string &path) {

    }

    void AssetManager::destroy(const AssetHandle &handle) {
        m_registry.destroy(handle.m_asset_id);
    }
}