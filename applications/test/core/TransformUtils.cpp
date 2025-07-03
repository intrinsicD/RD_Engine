#include "TransformUtils.h"
#include "Log.h"

#include <entt/entity/registry.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

namespace RDE::TransformUtils{
    glm::mat4 GetModelMatrix(const TransformParameters &parameters) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), parameters.translation);
        glm::mat4 rotation = glm::mat4_cast(parameters.orientation); // Convert quaternion to rotation matrix
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), parameters.scale);
        return translation * rotation * scaling;
    }

    TransformParameters DecomposeModelMatrix(const glm::mat4 &model_matrix) {
        TransformParameters parameters;
        // Extract translation
        glm::vec3 scale, skew;
        glm::vec4 perspective;
        if(!glm::decompose(model_matrix, parameters.scale, parameters.orientation, parameters.translation, skew, perspective)){
            // Handle decomposition failure
            RDE_CORE_ERROR("Failed to decompose model matrix: {}", glm::to_string(model_matrix));
        }
        return parameters;
    }

    void SetTransformDirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) || !registry.all_of<TransformLocal>(entity_id)) {
            return;
        }
        registry.emplace_or_replace<TransformDirty>(entity_id);
    }
}