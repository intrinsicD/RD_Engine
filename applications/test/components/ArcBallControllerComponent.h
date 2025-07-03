#pragma once

#include <glm/glm.hpp>

namespace RDE {
    struct ArcBallController {
        glm::vec3 m_target_world_space{}; // Target point the camera is looking at
        glm::vec3 m_last_point_3d{}; // Last point in 3D space
        glm::vec2 m_last_point_2d{}; // Last point in 2D screen space
        bool m_last_point_ok = false; // Flag to check if the last point is valid
    };
}