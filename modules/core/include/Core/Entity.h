// RDE_Project/modules/core/include/Core/Entity.h
#pragma once

#include "Core/Scene.h"
#include "Core/Log.h"
#include <entt/entt.hpp>

class Entity {
public:
    Entity() = default;

    Entity(entt::entity handle, Scene *scene);

    template<typename T, typename... Args>
    T &AddComponent(Args &&... args) {
        RDE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
        return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    template<typename T>
    T &GetComponent() {
        RDE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return m_scene->m_registry.get<T>(m_handle);
    }

    template<typename T>
    bool HasComponent() {
        return m_scene->m_registry.all_of<T>(m_handle);
    }

    template<typename T>
    void RemoveComponent() {
        RDE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        m_scene->m_registry.remove<T>(m_handle);
    }

    operator entt::entity() const { return m_handle; }

    operator uint32_t() const { return (uint32_t) m_handle; }

    bool operator==(const Entity &other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }

private:
    entt::entity m_handle{entt::null};
    Scene *m_scene = nullptr;
};