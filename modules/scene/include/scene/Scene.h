#pragma once

#include "assets/AssetDatabase.h"

#include <memory>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

namespace RDE {
    class Scene {
    public:
        Scene(std::unique_ptr<entt::registry> registry,
              std::unique_ptr<entt::dispatcher> dispatcher,
              AssetDatabase *asset_database = nullptr)
            : m_registry(std::move(registry)),
              m_dispatcher(std::move(dispatcher)),
              m_asset_database(asset_database) {
        };

        ~Scene() = default;

        entt::registry &get_registry() {
            return *m_registry;
        }

        const entt::registry &get_registry() const {
            return *m_registry;
        }

        entt::dispatcher &get_dispatcher() {
            return *m_dispatcher;
        }

        const entt::dispatcher &get_dispatcher() const {
            return *m_dispatcher;
        }

        bool instantiate(entt::entity entity_id, AssetID asset_id);

    private:
        std::unique_ptr<entt::registry> m_registry; // Entity registry for managing entities and components
        std::unique_ptr<entt::dispatcher> m_dispatcher; // Event dispatcher for handling events
        AssetDatabase *m_asset_database = nullptr; // Pointer to the asset database for loading assets
    };
}
