#pragma once

#include <glm/glm.hpp>

namespace RDE::Components {
    struct CameraComponent {
        glm::mat4 view_matrix = glm::mat4(1.0f);
        glm::mat4 projection_matrix = glm::mat4(1.0f);
    };
}
