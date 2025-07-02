#pragma once

#include "Plane.h"

#include <glm/glm.hpp>
#include <variant>

namespace RDE::Camera {
    struct Matrices {
        glm::mat4 view_matrix = glm::mat4(1.0f); // View matrix
        glm::mat4 projection_matrix = glm::mat4(1.0f);; // Projection matrix
    };

    struct ViewParameters {
        glm::vec3 position; // Camera position in world space
        glm::vec3 forward; // Forward direction vector
        glm::vec3 up; // Up direction vector

        glm::vec3 get_right() const {
            return glm::normalize(glm::cross(forward, up)); // Right direction vector
        }
    };

    struct ProjectionParameters {
        struct Perspective {
            float fov_degrees = 45.0f;
            float aspect_ratio = 16.0f / 9.0f;
        };
        struct Orthographic {
            float left = -1.0f;
            float right = 1.0f;
            float bottom = -1.0f;
            float top = 1.0f;
        };

        std::variant<Perspective, Orthographic> parameters { Perspective{} };
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
    };

    struct FrustumPlanes {
        Plane planes[6]; // Six planes defining the frustum (left, right, top, bottom, near, far)
    };

    struct Dirty {
    };

    struct Primary {
    };

    glm::mat4 CalculateViewMatrix(const ViewParameters &view_params);

    glm::mat4 CalculatePerspectiveProjectionMatrix(const ProjectionParameters::Perspective &perspective_params,
                                                   float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateOrthographicProjectionMatrix(const ProjectionParameters::Orthographic &orthographic_params,
                                                    float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateProjectionMatrix(const ProjectionParameters &projection_params);

    FrustumPlanes CalculateFrustumPlanes(const glm::mat4 &view_projection_matrix);

}