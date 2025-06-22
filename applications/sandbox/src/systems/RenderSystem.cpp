#include "RenderSystem.h"
#include "Scene.h"
#include "AssetManager.h"

#include "IRenderer.h"
#include "Entity.h"
#include "MeshAsset.h"
#include "MaterialAsset.h"

#include "EntityComponents/TransformComponent.h"
#include "EntityComponents/RenderableComponent.h"
#include "EntityComponents/CameraComponent.h"
#include "EntityComponents/PrimaryCameraTag.h"
#include "EntityComponents/CameraProjectionComponent.h"

namespace RDE{
    void RenderSystem::on_attach(Scene *scene) {

    }

    void RenderSystem::on_pre_update(Scene *scene, float delta_time) {

    }

    void RenderSystem::on_update(Scene *scene, float delta_time) {
        auto& registry = scene->get_registry();
        auto& renderer = scene->get_context().get<std::shared_ptr<IRenderer>>();
        auto& asset_manager = scene->get_context().get<AssetManager>();

        // 1. Get Camera data and begin the scene
        Entity camera_entity = get_primary_camera(scene);
        if (!camera_entity) return;

        renderer->begin_scene(
            camera_entity.get_component<TransformComponent>(),
            camera_entity.get_component<CameraProjectionComponent>(),
            camera_entity.get_component<CameraComponent>()
        );

        // 2. Build a list of draw commands
        std::vector<IRenderer::DrawCommand> draw_list;
        auto view = registry.view<const TransformComponent, const RenderableComponent>();

        for (auto entity : view) {
            const auto& transform = view.get<const TransformComponent>(entity);
            const auto& renderable = view.get<const RenderableComponent>(entity);

            // Resolve handles to get asset pointers
            MeshAsset* mesh = asset_manager.get<MeshAsset>(renderable.mesh_handle);
            MaterialAsset* material = asset_manager.get<MaterialAsset>(renderable.material_handle);

            if (mesh && material) {
                draw_list.emplace_back(IRenderer::DrawCommand{
                    .mesh = mesh,
                    .material = material,
                    .transform = transform.get_transform()
                });
            }
        }

        // 3. Submit the entire list to the renderer for drawing
        renderer->submit(draw_list);
        renderer->end_scene();
    }

    void RenderSystem::on_post_update(Scene *scene, float delta_time) {

    }

    void RenderSystem::on_event(Scene *scene, Event &e) {

    }

    void RenderSystem::set_renderer(Scene *scene, std::shared_ptr<IRenderer> renderer) {
        // This method is not strictly necessary if the renderer is always set in on_attach.
        // However, it can be useful for testing or dynamic changes.
        auto& context = scene->get_context();
        if (context.find<std::shared_ptr<IRenderer>>()) {
            context.erase<std::shared_ptr<IRenderer>>();
        }
        context.emplace<std::shared_ptr<IRenderer>>(renderer);
    }

    Entity RenderSystem::get_primary_camera(Scene *scene) {
        // This implementation is correct and idiomatic for EnTT.
        // It relies on the user ensuring only one entity has the PrimaryCameraTag.
        auto view = scene->get_registry().view<PrimaryCameraTag>();
        if (view.empty()) {
            return {entt::null, scene};
        }
        return {view.front(), scene};
    }
}