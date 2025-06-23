// RDE_Project/modules/core/include/Scene.h
#pragma once

#include <entt/entt.hpp>

namespace RDE {
    class Entity; // Forward declaration

    class Scene {
    public:
        Scene();

        ~Scene();

        Entity create_entity(const std::string &name = std::string());

        void destroy_entity(Entity entity);

        void on_update(float ts);

        void clear();

        entt::registry &get_registry() { return m_registry; }

        auto &get_context() {
            return m_registry.ctx();
        }

    private:
        entt::registry m_registry;

        friend class Entity;
    };
}