#include "CameraSystem.h"
#include "Scene.h"
#include "Input.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/ArcballControllerComponent.h"
#include <GLFW/glfw3.h>
#include <numbers>

namespace RDE::CameraSystem {
    // Helper function moved from the old EditorCamera class
    static bool MapToSphere(const ArcballControllerComponent &controller, const glm::vec2 &point, glm::vec3 &result,
                            const CameraComponent &cam_comp) {
        // This function would need the viewport size, which could be stored on the ArcballControllerComponent
        // or passed in. For now, let's assume it's stored.
        // ... implementation of map_to_sphere ...
        return true;
    }

    void UpdateArcball(Scene *scene, float delta_time) {
        auto view = scene->get_registry().view<TransformComponent, CameraComponent, ArcballControllerComponent>();
        for (auto entity_id: view) {
            auto [transform, camera, controller] = view.get<TransformComponent, CameraComponent,
                ArcballControllerComponent>(entity_id);

            const glm::vec2 &current_mouse = {Input::GetMouseX(), Input::GetMouseY()};
            glm::vec2 delta = current_mouse - controller.last_mouse_position;

            if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                if (!controller.is_rotating) {
                    controller.is_rotating = true;
                    MapToSphere(controller, current_mouse, controller.rotation_start_point_3d, camera);
                    controller.start_rotation = transform.rotation;
                } else {
                    // Rotation logic here, modifying controller & transform components
                    // ...
                }
            }
            // ... Pan logic for middle mouse ...
            else {
                controller.is_rotating = false;
                controller.is_panning = false;
            }

            controller.last_mouse_position = current_mouse;

            // Recalculate transform based on controller state
            // ...
        }
    }

    void OnEventArcball(Scene *scene, Event &e) {
        // Event handling logic for zoom, etc.
    }

    glm::mat4 GetViewProjection(Scene *scene) {
        // Find the primary camera and return its VP matrix
        // ...
        return glm::mat4(1.0f);
    }
}
