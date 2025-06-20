// file: Systems/CameraSystem.cpp
#include "CameraSystem.h"

// Core & Scene
#include "Scene.h"
#include "Entity.h"

// Events
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

// Context Components
#include "ContextComponents/MouseContextComponent.h"

// Entity Components
#include "EntityComponents/TransformComponent.h"
#include "EntityComponents/CameraComponent.h"
#include "EntityComponents/PerspectiveComponent.h"
#include "EntityComponents/OrthographicComponent.h"
#include "EntityComponents/PrimaryCameraTag.h"
#include "EntityComponents/ArcballControllerComponent.h"
#include "EntityComponents/DirtyTag.h"

// Libs
#include <glm/gtc/matrix_transform.hpp>
#include <entt/entity/entity.hpp>

namespace RDE {

    // Forward declaration for the controller logic helper.
    glm::quat GetArcBallRotation(ArcballControllerComponent &controller, glm::vec2 &new_point_2d);

    void RotateAroundFocalPoint(TransformComponent &transform, const glm::vec3 &focal_point, glm::quat rotation_delta);

    void CameraSystem::on_attach(Scene *scene) {
        // On attach, we ensure all cameras are considered "dirty" so their matrices
        // are correctly calculated on the first frame by the on_update loop.
        auto &registry = scene->get_registry();

        // Mark all projection-related components as dirty.
        for (auto entity: registry.view<PerspectiveComponent>()) {
            registry.emplace<DirtyTag<PerspectiveComponent>>(entity);
        }
        for (auto entity: registry.view<OrthographicComponent>()) {
            registry.emplace<DirtyTag<OrthographicComponent>>(entity);
        }

        // Mark all camera transforms as dirty to initialize the view matrix.
        for (auto entity: registry.view<CameraComponent, TransformComponent>()) {
            registry.emplace<DirtyTag<TransformComponent>>(entity);
        }
    }

    void CameraSystem::on_update(Scene *scene, float delta_time) {
        auto &registry = scene->get_registry();

        // Step 1: Run camera controllers
        // This will modify TransformComponents and mark them as dirty.
        {
            auto primary_camera_entity = get_primary_camera_entity(scene);
            if (primary_camera_entity) {
                if(scene->get_registry().all_of<ArcballControllerComponent, TransformComponent>(primary_camera_entity)){
                    auto &arcball_controller = scene->get_registry().get<ArcballControllerComponent>(primary_camera_entity);
                    auto &transform = scene->get_registry().get<TransformComponent>(primary_camera_entity);

                    auto &mouse_context = scene->get_registry().ctx().get<MouseContextComponent>();
                    glm::quat rotation_delta = GetArcBallRotation(arcball_controller, mouse_context.position_delta);
                    RotateAroundFocalPoint(transform, arcball_controller.focal_point, rotation_delta);

                    registry.emplace_or_replace<DirtyTag<TransformComponent>>(primary_camera_entity);
                }
            }
        }

        // Step 2: Update projection matrices for dirty perspective cameras
        {
            auto view = registry.view<CameraComponent, PerspectiveComponent, DirtyTag<PerspectiveComponent>>();
            for (auto entity: view) {
                auto &camera = view.get<CameraComponent>(entity);
                auto &perspective = view.get<PerspectiveComponent>(entity);

                camera.projection_matrix = glm::perspective(glm::radians(perspective.fovy_degrees),
                                                            perspective.aspect_ratio,
                                                            perspective.zNear,
                                                            perspective.zFar);
            }
            // Clean the dirty tags after processing.
            registry.clear<DirtyTag<PerspectiveComponent>>();
        }

        // Step 3: Update projection matrices for dirty orthographic cameras
        {
            auto view = registry.view<CameraComponent, OrthographicComponent, DirtyTag<OrthographicComponent>>();
            for (auto entity: view) {
                auto &camera = view.get<CameraComponent>(entity);
                auto &ortho = view.get<OrthographicComponent>(entity);
                camera.projection_matrix = glm::ortho(ortho.left, ortho.right,
                                                      ortho.bottom, ortho.top,
                                                      ortho.zNear, ortho.zFar);
            }
            // Clean the dirty tags after processing.
            registry.clear<DirtyTag<OrthographicComponent>>();
        }

        // Step 4: Update view matrices for cameras with dirty transforms
        {
            // NOTE: This includes cameras whose transforms were just modified by controllers.
            auto view = registry.view<CameraComponent, TransformComponent, DirtyTag<TransformComponent>>();
            for (auto entity: view) {
                auto &camera = view.get<CameraComponent>(entity);
                auto &transform = view.get<TransformComponent>(entity);
                camera.view_matrix = glm::inverse(transform.get_transform());
            }
            // Clean the dirty tags after processing.
            registry.clear<DirtyTag<TransformComponent>>();
        }
    }

    void CameraSystem::on_event(Scene *scene, Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>([&](WindowResizeEvent &event) -> bool {
            if (event.get_width() == 0 || event.get_height() == 0) {
                return false;
            }

            auto &registry = scene->get_registry();
            const float aspect_ratio = static_cast<float>(event.get_width()) / static_cast<float>(event.get_height());

            // Update all perspective cameras with the new aspect ratio and mark them dirty.
            auto perspective_view = registry.view<PerspectiveComponent>();
            for (auto entity: perspective_view) {
                auto &perspective_component = perspective_view.get<PerspectiveComponent>(entity);
                perspective_component.aspect_ratio = aspect_ratio;
                registry.emplace_or_replace<DirtyTag<PerspectiveComponent>>(entity);
            }

            // Update all orthographic cameras and mark them dirty.
            auto orthographic_view = registry.view<OrthographicComponent>();
            for (auto entity: orthographic_view) {
                auto &ortho_component = orthographic_view.get<OrthographicComponent>(entity);
                // Example: maintain a constant view height
                // float zoom_level = 1.0f;
                // ortho_component.left = -aspect_ratio * zoom_level;
                // ortho_component.right = aspect_ratio * zoom_level;
                registry.emplace_or_replace<DirtyTag<OrthographicComponent>>(entity);
            }
            return false; // Don't consume the event
        });
    }

    Entity CameraSystem::get_primary_camera_entity(Scene *scene) const {
        // This implementation is correct and idiomatic for EnTT.
        // It relies on the user ensuring only one entity has the PrimaryCameraTag.
        auto view = scene->get_registry().view<PrimaryCameraTag>();
        if (view.empty()) {
            return {entt::null, scene};
        }
        return {view.front(), scene};
    }

    // --- Helper Implementation ---

    bool MapToSphere(const glm::vec2 &point, int width, int height, glm::vec3 &point_on_sphere) {
        if ((point[0] >= 0) && (point[0] <= width) && (point[1] >= 0) && (point[1] <= height)) {
            double x = (double) (point[0] - 0.5 * width) / width;
            double y = (double) (0.5 * height - point[1]) / height;
            double sinx = sin(std::numbers::pi * x * 0.5);
            double siny = sin(std::numbers::pi * y * 0.5);
            double sinx2siny2 = sinx * sinx + siny * siny;

            point_on_sphere[0] = sinx;
            point_on_sphere[1] = siny;
            point_on_sphere[2] = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;

            return true;
        } else
            return false;
    }

    glm::quat GetArcBallRotation(ArcballControllerComponent &controller, glm::vec2 &new_point_2d) {
        if (controller.last_point_ok) {
            glm::vec3 new_point_3D;
            bool newPointok = MapToSphere(new_point_2d, controller.width, controller.height, new_point_3D);

            if (newPointok) {
                glm::vec3 axis = cross(controller.last_point_3d, new_point_3D);
                float cosAngle = glm::dot(controller.last_point_3d, new_point_3D);

                if (fabs(cosAngle) < 1.0) {
                    float angle_radians = acos(cosAngle);
                    return glm::angleAxis(angle_radians, glm::normalize(axis));
                }
            }
        }
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion if no rotation
    }

    void RotateAroundFocalPoint(TransformComponent &transform, const glm::vec3 &focal_point, glm::quat rotation_delta) {
        glm::vec3 offset = transform.position - focal_point;
        transform.position = focal_point + (rotation_delta * offset);
        transform.rotation = rotation_delta * transform.rotation;
        transform.rotation = glm::normalize(transform.rotation); // Normalize to prevent drift
    }
}