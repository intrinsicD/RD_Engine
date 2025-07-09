#pragma once

#include <memory>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

namespace RDE{
    class Scene {
    public:
        Scene() = default;

        ~Scene() = default;

        void set_registry(std::unique_ptr<entt::registry> registry) {
            m_registry = std::move(registry);
        }

        void set_dispatcher(std::unique_ptr<entt::dispatcher> dispatcher) {
            m_dispatcher = std::move(dispatcher);
        }

        void init(){
            if(!m_registry){
                m_registry = std::make_unique<entt::registry>();
            }
            if(!m_dispatcher){
                m_dispatcher = std::make_unique<entt::dispatcher>();
            }
        }

        entt::registry &get_registry() {
            return *m_registry;
        }

        entt::dispatcher &get_dispatcher() {
            return *m_dispatcher;
        }
    private:
        std::unique_ptr<entt::registry> m_registry; // Entity registry for managing entities and components
        std::unique_ptr<entt::dispatcher> m_dispatcher; // Event dispatcher for handling events
    };
}