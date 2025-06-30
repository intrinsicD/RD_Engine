// RDE_Project/modules/core/include/Entity.h
#pragma once

#include "Scene.h"
#include "Log.h"

#include <entt/entt.hpp>

namespace RDE {
    class Entity {
    public:
        Entity() = default;

        Entity(entt::entity handle, Scene *scene) : m_handle(handle), m_scene(scene) {
            RDE_CORE_ASSERT(m_scene, "Scene cannot be null!");
            RDE_CORE_ASSERT(m_handle != entt::null, "Entity handle cannot be null!");
        }

        template<typename T, typename... Args>
        T &add_component(Args &&... args) {
            RDE_CORE_ASSERT(!has_component<T>(), "Entity already has component!");
            return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
        }

        template<typename T>
        T &get_component() {
            RDE_CORE_ASSERT(has_component<T>(), "Entity does not have component!");
            return m_scene->m_registry.get<T>(m_handle);
        }

        template<typename T>
        bool has_component() {
            return m_scene->m_registry.all_of<T>(m_handle);
        }

        template<typename T>
        void remove_component() {
            RDE_CORE_ASSERT(has_component<T>(), "Entity does not have component!");
            m_scene->m_registry.remove<T>(m_handle);
        }

        operator entt::entity() const { return m_handle; }

        operator uint32_t() const { return (uint32_t) m_handle; }

        bool operator==(const Entity &other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }

        bool operator!=(const Entity &other) const { return !(*this == other); }

        operator bool() const { return m_handle != entt::null; }

    private:
        entt::entity m_handle{entt::null};
        Scene *m_scene = nullptr;
    };
}