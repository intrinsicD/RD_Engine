#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RDE {
    struct ArcballControllerComponent {
        glm::vec3 focal_point{0.0f, 0.0f, 0.0f};
        float distance = 10.0f;

        int width = 800; // Default width
        int height = 600; // Default height

        glm::vec2 last_point_2d;
        glm::vec3 last_point_3d;
        bool last_point_ok = false;
    };
}
