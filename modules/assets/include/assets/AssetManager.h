// assets/AssetManager.h
#pragma once

#include "AssetHandle.h"
#include "AssetDatabase.h"
#include "ILoader.h"
#include "core/Log.h"

#include <entt/resource/cache.hpp>
#include <string>
#include <functional>
#include <typeindex>
#include <filesystem>

namespace RDE {
    class AssetManager {
    public:
        explicit AssetManager(AssetDatabase &asset_database) : m_database(asset_database) {}

        ~AssetManager() = default;

        void register_loader(const std::shared_ptr<ILoader> &loader) {
            for (const auto &ext: loader->get_supported_extensions()) {
                m_loaders[ext] = loader;
            }
        }

        AssetID load(const std::string &uri) {
            auto it_cache = m_cache.find(uri);

            if (it_cache != m_cache.end()) {
                RDE_CORE_TRACE("Asset Cache HIT for '{}'.", uri);
                // Return the cached AssetID as an AssetHandle.
                return it_cache->second;
            }

            RDE_CORE_TRACE("Asset Cache MISS for '{}'. Proceeding to load.", uri);

            //get extension from uri
            std::filesystem::path path(uri);
            std::string ext = path.extension().string();
            auto it_loader = m_loaders.find(ext);
            if (it_loader == m_loaders.end()) {
                throw std::runtime_error("AssetManager: No loader registered for this asset concept.");
            }

            auto &loader = it_loader->second;
            RDE_CORE_TRACE("Using loader for '{}': {}", uri, ext);
            auto new_asset_id = loader->load(uri, m_database);

            if (!new_asset_id) {
                RDE_CORE_ERROR("Failed to load asset from '{}'.", uri);
                return nullptr;
            }

            m_cache[uri] = new_asset_id;
            RDE_CORE_TRACE("Asset loaded successfully. Entity ID: {}, URI: {}", static_cast<uint32_t>(new_asset_id->entity_id), uri);

            return new_asset_id;
        }


        AssetID force_load(const std::string &uri) {
            RDE_CORE_INFO("Force loading asset from '{}'.", uri);
            // Clear the cache for this URI to force reload.
            m_cache.erase(uri);

            // Call the regular load function to reload the asset.
            return load(uri);
        }

        AssetDatabase &get_database() {
            return m_database;
        }
    private:
        AssetDatabase &m_database;

        // Your runtime-pluggable system.
        std::unordered_map<std::string, AssetID> m_cache;
        std::unordered_map<std::string, std::shared_ptr<ILoader>> m_loaders;
    };
}