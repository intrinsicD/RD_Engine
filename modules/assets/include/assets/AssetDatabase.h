//assets/AssetDatabase.h
#pragma once

#include "assets/AssetHandle.h"
#include <entt/entity/registry.hpp>

namespace RDE {
    class AssetDatabase {
    public:
        AssetDatabase() = default;

        ~AssetDatabase() = default;

        template<typename AssetComponentType, typename AssetConcept>
        AssetComponentType &get(AssetHandle<AssetConcept> &handle) {
            if (!handle) throw std::runtime_error("Attempted to use an invalid asset handle.");
            return get<AssetComponentType>(handle.internal_handle);
        }

        template<typename AssetComponentType>
        AssetComponentType &get(const AssetID& asset_id) {
            if (!m_registry.valid(asset_id->entity_id)) {
                throw std::runtime_error("Attempted to access an invalid entity in the asset database.");
            }
            return m_registry.get<AssetComponentType>(asset_id->entity_id);
        }

        // try_get version for safe access
        template<typename AssetComponentType, typename AssetConcept>
        AssetComponentType *try_get(AssetHandle<AssetConcept> &handle) {
            if (!handle) {
                return nullptr;
            }
            return try_get(handle.internal_handle);
        }

        template<typename AssetComponentType>
        AssetComponentType *try_get(const AssetID& asset_id) {
            if (!m_registry.valid(asset_id->entity_id)) {
                return nullptr;
            }
            const entt::entity entity = asset_id->entity_id;
            if (!m_registry.valid(entity)) {
                return nullptr;
            }
            return m_registry.try_get<AssetComponentType>(entity);
        }

        entt::registry &get_registry() {
            return m_registry;
        }

        const entt::registry &get_registry() const {
            return m_registry;
        }

    private:
        friend class AssetManager;

        entt::entity create_asset() {
            // Create a new entity in the registry for the asset
            return m_registry.create();
        }

        void destroy_asset(entt::entity entity) {
            m_registry.destroy(entity);
        }

        template<typename... Args>
        auto emplace(entt::entity entity, Args &&... args) {
            return m_registry.emplace<Args...>(entity, std::forward<Args>(args)...);
        }

        template<typename... Args>
        auto emplace_or_replace(entt::entity entity, Args &&... args) {
            return m_registry.emplace_or_replace<Args...>(entity, std::forward<Args>(args)...);
        }

        entt::registry m_registry;
    };
}
