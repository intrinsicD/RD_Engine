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

#include <entt/fwd.hpp>

namespace RDE::TransformUtils{
    using TransformParameters = TransformLocal;

    glm::mat4 GetModelMatrix(const TransformParameters &transform);

    TransformParameters DecomposeModelMatrix(const glm::mat4 &model_matrix);

    void SetTransformDirty(entt::registry &registry, entt::entity entity_id);
}
