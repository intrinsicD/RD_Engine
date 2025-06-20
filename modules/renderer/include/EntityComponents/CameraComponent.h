#pragma once

#include <glm/glm.hpp>

namespace RDE {
    struct CameraComponent {
        glm::mat4 projection_matrix = glm::mat4(1.0f); // Projection matrix
        glm::mat4 view_matrix = glm::mat4(1.0f); // Projection matrix
    };
}