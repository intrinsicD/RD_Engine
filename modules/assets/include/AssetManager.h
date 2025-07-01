#pragma once

#include "AssetHandle.h"
#include "AssetComponentTypes.h"
#include "AssetDatabase.h"
#include "Log.h"
#include "internal/AssetCreationReceipt.h"

#include <any>
#include <typeindex>
#include <filesystem>
#include <entt/entity/registry.hpp>
#include <entt/resource/cache.hpp>
#include <entt/resource/resource.hpp>

namespace RDE {
    class AssetManager {
    public:
        explicit AssetManager(AssetDatabase &asset_database): m_database(asset_database) {
        }

        ~AssetManager() = default;

        template<typename AssetConcept, typename LoaderType>
        void register_loader(const std::vector<std::string> &supported_extensions) {
            const auto concept_type_id = std::type_index(typeid(AssetConcept));

            m_cache_factories[concept_type_id] = [this]() {
                return std::make_any<entt::resource_cache<AssetCreationReceipt> >(std::make_unique<LoaderType>());
            };

            m_generic_reload_funcs[concept_type_id] = [this](const std::string &uri) {
                RDE_CORE_TRACE("AssetManager: Force reloading asset type '{}' from URI '{}'", typeid(AssetConcept).name(),
                               uri);

                auto &cache = this->get_or_create_cache<AssetConcept>();
                cache.force_load(uri, std::ref(m_database));
            };

            for (const auto &extension : supported_extensions) {
                m_extension_to_type_map[extension] = concept_type_id;
            }
        }

        template<typename AssetConcept>
        AssetHandle<AssetConcept> load(const std::string &uri) {
            if (auto it = m_handle_cache.find(uri); it != m_handle_cache.end()) {
                if (AssetID locked_id = it->second.lock()) {
                    RDE_CORE_INFO("Asset Handle Cache HIT for '{}'.", uri);
                    return AssetHandle<AssetConcept>(locked_id);
                }
            }

            RDE_CORE_INFO("Asset Handle Cache MISS for '{}'. Proceeding to data cache.", uri);

            auto &data_cache = get_or_create_cache<AssetConcept>();
            entt::resource<AssetCreationReceipt> resource_handle = data_cache.load(uri, std::ref(m_database));

            if (!resource_handle) {
                RDE_CORE_ERROR("Failed to load asset from URI: {}", uri);
                return AssetHandle<AssetConcept>(); // Return invalid handle
            }

            entt::entity asset_entity = resource_handle->entity_id;

            auto deleter = [db = &this->m_database](AssetID_Data *data) {
                db->destroy_asset(data->entity_id);
                delete data;
                RDE_CORE_INFO("Final handle for asset '{}' released. Entity destroyed.", data->uri);
            };

            auto data = new AssetID_Data{asset_entity, uri};
            AssetID new_id(data, deleter);

            m_handle_cache[uri] = new_id;

            return AssetHandle<AssetConcept>(new_id);
        }

        bool force_load_from(const std::string& uri, std::function<void(std::string)> reload_callback = nullptr) {
            // 1. Determine type from file extension
            std::string extension = std::filesystem::path(uri).extension().string();
            auto it = m_extension_to_type_map.find(extension);
            if (it == m_extension_to_type_map.end()) {
                RDE_CORE_WARN("Hot Reload: No asset type registered for extension '{}'", extension);
                return false;
            }
            const std::type_index& type_id = it->second;

            // 2. We need a way to trigger the correct generic function.
            //    This requires a bit of type-erasure magic.
            auto cache_it = m_generic_reload_funcs.find(type_id);
            if (cache_it == m_generic_reload_funcs.end()) {
                RDE_CORE_ERROR("Hot Reload: No reload function found for type '{}'", type_id.name());
                return false;
            }

            // 3. Call the stored lambda to reload the asset
            cache_it->second(uri);
            if (reload_callback) {
                reload_callback(uri);
            }
            return true;
        }

    private:
        template<typename AssetConcept>
        entt::resource_cache<AssetCreationReceipt> &get_or_create_cache() {
            const auto type_id = std::type_index(typeid(AssetConcept));

            auto it = m_caches.find(type_id);
            if (it == m_caches.end()) {
                auto factory_it = m_cache_factories.find(type_id);
                if (factory_it == m_cache_factories.end()) {
                    throw std::runtime_error(
                        "AssetManager: No loader registered for asset type " + std::string(typeid(AssetConcept).name()));
                }

                std::any new_cache_any = factory_it->second();

                it = m_caches.emplace(type_id, std::move(new_cache_any)).first;
                RDE_CORE_INFO("AssetManager: Created new cache for asset type '{}'.", typeid(AssetConcept).name());
            }

            return std::any_cast<entt::resource_cache<AssetCreationReceipt> &>(it->second);
        }

        AssetDatabase &m_database;
        std::unordered_map<std::type_index, std::any> m_caches;
        std::unordered_map<std::type_index, std::function<std::any()> > m_cache_factories;
        std::unordered_map<std::string, std::weak_ptr<AssetID_Data> > m_handle_cache;
        std::unordered_map<std::type_index, std::function<void(const std::string &)> > m_generic_reload_funcs;
        std::unordered_map<std::string, std::type_index> m_extension_to_type_map;
    };
}
