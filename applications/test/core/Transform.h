#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    struct TransformModelMatrix {
        glm::mat4 model_matrix = glm::mat4(1.0f); // Default to identity matrix
    };

    struct TransformParameters {
        glm::vec3 translation = {0.0f, 0.0f, 0.0f}; // Position in world space
        glm::quat orientation = {1.0f, 0.0f, 0.0f, 0.0f}; // Quaternion representing rotation
        glm::vec3 scale = {1.0f, 1.0f, 1.0f}; // Scale factors along each axis
    };

    struct DirtyTransform {
    };

    glm::mat4 get_model_matrix(const TransformParameters &transform);

    TransformParameters decompose_model_matrix(const glm::mat4 &model_matrix);
}