#include "systems/RenderSystem.h"
#include "Application.h"
#include "IRenderer.h"
#include "Scene.h"
#include "Log.h"

#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "components/IsPrimaryTag.h"
#include "components/IsVisibleTag.h"

namespace RDE {
    void RenderSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("RenderSystem attached");
    }

    void RenderSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("RenderSystem detached");
    }

    CameraData GetPrimaryCameraData(Scene *scene) {
        auto &registry = scene->get_registry();
        auto view = registry.view<Components::CameraComponent, Components::IsPrimaryTag<Components::CameraComponent>>();

        if (view.size_hint() == 0) {
            return {.view = glm::mat4(1.0f),
                    .projection = glm::mat4(1.0f)}; // Return a default camera data
        }

        // Assuming there's only one primary camera
        auto entity = view.front();
        auto &camera_component = registry.get<Components::CameraComponent>(entity);
        auto &transform = registry.get<Components::Transform>(entity);
        // Construct the camera data from the component
        CameraData camera_data;
        camera_data.view = glm::inverse(transform.model_matrix);
        camera_data.projection = camera_component.projection_matrix;
        return camera_data;
    }

    void RenderSystem::on_pre_update(Scene *scene, float delta_time) {
        // This system does not require pre-update logic
        // but can be used to prepare any state before the main update.

        Application &app = Application::get();
        auto &renderer = app.get_renderer();
        // Prepare the renderer for the frame
        //Collect all visible, renderable entities, prepare render queues, etc.

        {
            auto &registry = scene->get_registry();
            auto view = registry.view<Components::IsVisibleTag>();
            for (auto entity : view) {
                // Here you can collect all visible entities and prepare them for rendering
                // For example, you might want to gather their geometry and material handles
                // or any other data needed for rendering.
                // This is a good place to set up render queues or similar structures.

            }
        }
    }

    void RenderSystem::on_update(Scene *scene, float delta_time) {
        Application &app = Application::get();
        auto &renderer = app.get_renderer();

        // Begin the frame
        renderer.begin_frame();

        // Draw the frame with the camera data
        auto camera_data = GetPrimaryCameraData(scene);
        renderer.draw_frame(camera_data);

        // End the frame
        renderer.end_frame();
    }
}