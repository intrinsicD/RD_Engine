#include "systems/TransformSystem.h"
#include "components/TransformComponent.h"
#include "components/DirtyTag.h"
#include "Scene.h"
#include "Entity.h"

namespace RDE {
    // The function that gets called by the observer
    void on_transform_updated(entt::registry &registry, entt::entity entity) {
        // Add the dirty tag so the TransformSystem will pick it up next frame.
        registry.emplace_or_replace<Components::DirtyTag<Components::Transform> >(entity);
    }

    void TransformSystem::on_attach(Scene *scene) {
        scene->get_registry().on_update<Components::Transform>().connect<&on_transform_updated>();
        scene->get_registry().on_construct<Components::Transform>().connect<&on_transform_updated>();

        auto view = scene->get_registry().view<Components::Transform>(
            entt::exclude<Components::DirtyTag<Components::Transform> >);

        for (const auto entity_id: view) {
            Entity entity(entity_id, scene);
            entity.add_component<Components::DirtyTag<Components::Transform> >();
        }
    }

    void TransformSystem::on_detach(Scene *scene) {
        scene->get_registry().on_update<Components::Transform>().disconnect<&on_transform_updated>();
        scene->get_registry().on_construct<Components::Transform>().disconnect<&on_transform_updated>();

        auto view = scene->get_registry().view<Components::DirtyTag<Components::Transform> >();
        for (const auto entity_id: view) {
            Entity entity(entity_id, scene);
            entity.remove_component<Components::DirtyTag<Components::Transform> >();
        }
    }

    void TransformSystem::on_update(Scene *scene, float delta_time) {
        auto &registry = scene->get_registry();
        auto view = registry.view<Components::DirtyTag<Components::Transform>, Components::Transform>();

        for (const auto entity_id: view) {
            auto &transform = registry.get<Components::Transform>(entity_id);
            transform.model_matrix = glm::translate(glm::mat4(1.0f), transform.position) *
                                     glm::mat4_cast(transform.rotation) *
                                     glm::scale(glm::mat4(1.0f), transform.scale);
            registry.remove<Components::DirtyTag<Components::Transform> >(entity_id);
        }
    }
}
