// file: EntityComponents/CameraComponent.h
#pragma once

#include <glm/glm.hpp>

namespace RDE {
    // This component holds cached matrix data derived from a camera's
    // TransformComponent and CameraProjectionComponent. It is updated by the CameraSystem.
    struct CameraComponent {
        glm::mat4 projection_matrix = glm::mat4(1.0f);
        glm::mat4 view_matrix = glm::mat4(1.0f);
    };
}