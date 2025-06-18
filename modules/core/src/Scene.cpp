// RDE_Project/modules/core/src/Scene.cpp
#include "Core/Scene.h"
#include "Core/Entity.h"
#include "Core/Components.h"

Scene::Scene() {}

Scene::~Scene() {}

Entity Scene::CreateEntity(const std::string &name) {
    Entity entity = {m_registry.create(), this};
    // We will add a TagComponent here later

    entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
    return entity;
}

void Scene::OnUpdate(float ts) {
    // This is where we will run our "systems" later.
    // For now, it's empty.
}