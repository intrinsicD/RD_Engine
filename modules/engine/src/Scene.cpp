#include "Scene.h"
#include "Entity.h"
#include "components/NameTagComponent.h"

#include "systems/AnimationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CullingSystem.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

namespace RDE {
    Scene::Scene() {
        // Initialize the scene with a default camera.
        m_systems.emplace_back(std::make_unique<InputSystem>());

        // Add systems in the order they should be processed.

        // TransformSystem must be first to ensure all entities have their transforms updated before any other system.
        m_systems.emplace_back(std::make_unique<TransformSystem>());
        // AnimationSystem must be before PhysicsSystem to ensure animations are updated before physics calculations.
        m_systems.emplace_back(std::make_unique<AnimationSystem>());
        // PhysicsSystem must be after TransformSystem to ensure transforms are updated before physics calculations.
        m_systems.emplace_back(std::make_unique<PhysicsSystem>());
        // CameraSystem must be after TransformSystem to ensure camera matrices are updated correctly.
        m_systems.emplace_back(std::make_unique<CameraSystem>());
        m_systems.emplace_back(std::make_unique<CullingSystem>());
        m_systems.emplace_back(std::make_unique<RenderSystem>());
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