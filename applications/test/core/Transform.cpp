#include "Transform.h"
#include "Log.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

namespace RDE{
    glm::mat4 get_model_matrix(const TransformParameters &transform) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform.translation);
        glm::mat4 rotation = glm::mat4_cast(transform.orientation); // Convert quaternion to rotation matrix
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), transform.scale);
        return translation * rotation * scaling;
    }

    TransformParameters decompose_model_matrix(const glm::mat4 &model_matrix) {
        TransformParameters transform;
        // Extract translation
        glm::vec3 scale, skew;
        glm::vec4 perspective;
        if(!glm::decompose(model_matrix, transform.scale, transform.orientation, transform.translation, skew, perspective)){
            // Handle decomposition failure
            RDE_CORE_ERROR("Failed to decompose model matrix: {}", glm::to_string(model_matrix));
        }
        return transform;
    }
}