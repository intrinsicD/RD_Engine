#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    struct TransformDirty{};

    struct TransformLocal{
        glm::vec3 translation = {0.0f, 0.0f, 0.0f}; // Position in world space
        glm::quat orientation = {1.0f, 0.0f, 0.0f, 0.0f}; // Quaternion representing rotation
        glm::vec3 scale = {1.0f, 1.0f, 1.0f}; // Scale factors along each axis
    };

    struct TransformWorld {
        glm::mat4 matrix = glm::mat4(1.0f);
    };
}
