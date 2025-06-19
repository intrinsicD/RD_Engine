#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RDE {
    struct ArcballControllerComponent {
        glm::vec3 focal_point{0.0f, 0.0f, 0.0f};
        float distance = 10.0f;

        // Internal state for drag calculations
        bool is_rotating = false;
        bool is_panning = false;
        glm::quat start_rotation;
        glm::vec3 rotation_start_point_3d;
        glm::vec2 last_mouse_position;

        ArcballControllerComponent() = default;

        ArcballControllerComponent(const ArcballControllerComponent &) = default;
    };
}
