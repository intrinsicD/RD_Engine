#include "Scene.h"
#include "Entity.h"
#include "components/NameTagComponent.h"

namespace RDE {
    Scene::Scene() {

    }

    Scene::~Scene() {
        m_systems.clear();
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

    void Scene::on_update_simulation(float ts) {
        // This is where we will run our "systems" later.
        // For now, it's empty.
    }

    void Scene::on_update_presentation(float delta_time) {

    }

    void Scene::on_submit_render_data() {

    }

    void Scene::clear() {
        m_registry.clear(); // Clear all entities and components
    }

    bool Scene::detach_system(ISystem *system) {
        if (system) {
            system->on_detach();
        }
        auto it = std::remove_if(m_systems.begin(), m_systems.end(),
                                 [system](const std::unique_ptr<ISystem> &s) { return s.get() == system; });
        if (it != m_systems.end()) {
            m_systems.erase(it, m_systems.end());
            return true;
        } else {
            RDE_CORE_ERROR("System not found in the engine.");
            return false;
        }
    }

}