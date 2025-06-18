// RDE_Project/modules/core/include/Core/Scene.h
#pragma once

#include <entt/entt.hpp>

class Entity; // Forward declaration

class Scene {
public:
    Scene();

    ~Scene();

    Entity CreateEntity(const std::string &name = std::string());

    void OnUpdate(float ts);

    entt::registry &GetRegistry() { return m_registry; }

private:
    entt::registry m_registry;

    friend class Entity;
};