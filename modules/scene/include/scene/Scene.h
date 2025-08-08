#pragma once

#include "assets/AssetDatabase.h"
#include "scene/SystemScheduler.h"

#include <memory>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

namespace RDE {
    class Scene {
    public:
        Scene(AssetDatabase *asset_database = nullptr)
            : m_asset_database(asset_database) {
        };

        ~Scene() = default;

        entt::registry &get_registry() {
            return m_registry;
        }

        const entt::registry &get_registry() const {
            return m_registry;
        }

        entt::dispatcher &get_dispatcher() {
            return m_dispatcher;
        }

        const entt::dispatcher &get_dispatcher() const {
            return m_dispatcher;
        }

        SystemScheduler &get_system_scheduler() {
            return m_system_scheduler;
        }

        const SystemScheduler &get_system_scheduler() const {
            return m_system_scheduler;
        }

        void shutdown() {
            // Perform any necessary cleanup before the scene is destroyed
            m_system_scheduler.shutdown();
            m_registry.clear(); // Clear the registry to remove all entities and components
        }
    private:
        entt::registry m_registry; // Entity registry for managing entities and components
        entt::dispatcher m_dispatcher; // Event dispatcher for handling events
        SystemScheduler m_system_scheduler; // System scheduler for managing systems in the scene
        [[maybe_unused]] AssetDatabase *m_asset_database = nullptr; // Pointer to the asset database for loading assets
    };
}
