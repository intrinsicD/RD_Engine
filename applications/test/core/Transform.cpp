#include "Transform.h"
#include "Log.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

namespace RDE::Transform{
    glm::mat4 get_model_matrix(const Parameters &parameters) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), parameters.translation);
        glm::mat4 rotation = glm::mat4_cast(parameters.orientation); // Convert quaternion to rotation matrix
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), parameters.scale);
        return translation * rotation * scaling;
    }

    Parameters decompose_model_matrix(const glm::mat4 &model_matrix) {
        Parameters parameters;
        // Extract translation
        glm::vec3 scale, skew;
        glm::vec4 perspective;
        if(!glm::decompose(model_matrix, parameters.scale, parameters.orientation, parameters.translation, skew, perspective)){
            // Handle decomposition failure
            RDE_CORE_ERROR("Failed to decompose model matrix: {}", glm::to_string(model_matrix));
        }
        return parameters;
    }
}