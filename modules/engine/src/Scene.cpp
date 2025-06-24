#include "Scene.h"
#include "Entity.h"
#include "components/NameTagComponent.h"

namespace RDE {
    Scene::Scene() {
    }

    Scene::~Scene() {
    }

    Entity Scene::create_entity(const std::string &name) {
        Entity entity = {m_registry.create(), this};
        // We will add a TagComponent here later

        entity.add_component<Components::NameTagComponent>(name.empty() ? "Entity" : name);
        return entity;
    }

    void Scene::destroy_entity(Entity entity) {
        // We can use the registry to destroy the entity
        m_registry.destroy(entity);
    }

    void Scene::on_update(float ts) {
        // This is where we will run our "systems" later.
        // For now, it's empty.
    }

    void Scene::clear() {
        m_registry.clear(); // Clear all entities and components
    }

}