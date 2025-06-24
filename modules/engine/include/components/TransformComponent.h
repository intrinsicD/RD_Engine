#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RDE::Components {
    struct Transform {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        glm::vec3 scale = glm::vec3(1.0f);

        glm::mat4 model_matrix = glm::mat4(1.0f); // Default to identity matrix
    };
}
