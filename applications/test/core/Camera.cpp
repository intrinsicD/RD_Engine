#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RDE::Camera {
    glm::mat4 CalculateViewMatrix(const ViewParameters &view_params) {
        return glm::lookAt(view_params.position,
                           view_params.position + view_params.forward,
                           view_params.up);
    }

    glm::mat4 CalculatePerspectiveProjectionMatrix(const ProjectionParameters::Perspective &params,
                                                   float near_plane, float far_plane) {
        return glm::perspective(glm::radians(params.fov_degrees),
                                params.aspect_ratio,
                                near_plane,
                                far_plane);
    }

    glm::mat4 CalculateOrthographicProjectionMatrix(const ProjectionParameters::Orthographic &params,
                                                    float near_plane, float far_plane) {
        return glm::ortho(params.left, params.right,
                          params.bottom, params.top,
                          near_plane, far_plane);
    }

    glm::mat4 CalculateProjectionMatrix(const ProjectionParameters &params) {
        // Check if the parameters are of type Perspective or Orthographic
        if (std::holds_alternative<ProjectionParameters::Perspective>(params.parameters)) {
            const auto &perspective_params = std::get<ProjectionParameters::Perspective>(params.parameters);
            return CalculatePerspectiveProjectionMatrix(perspective_params, params.near_plane, params.far_plane);
        } else if (std::holds_alternative<ProjectionParameters::Orthographic>(params.parameters)) {
            const auto &orthographic_params = std::get<ProjectionParameters::Orthographic>(params.parameters);
            return CalculateOrthographicProjectionMatrix(orthographic_params, params.near_plane, params.far_plane);
        } else {
            return glm::mat4(1.0f); // Return identity matrix if no valid parameters are found
        }
    }

    FrustumPlanes CalculateFrustumPlanes(const glm::mat4 &m) {
        FrustumPlanes frustum;

        // Left Plane
        frustum.planes[0].normal = glm::vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]);
        frustum.planes[0].distance = m[3][3] + m[3][0];

        // Right Plane
        frustum.planes[1].normal = glm::vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]);
        frustum.planes[1].distance = m[3][3] - m[3][0];

        // Bottom Plane
        frustum.planes[2].normal = glm::vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]);
        frustum.planes[2].distance = m[3][3] + m[3][1];

        // Top Plane
        frustum.planes[3].normal = glm::vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]);
        frustum.planes[3].distance = m[3][3] - m[3][1];

        // Near Plane
        frustum.planes[4].normal = glm::vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]);
        frustum.planes[4].distance = m[3][3] + m[3][2];

        // Far Plane
        frustum.planes[5].normal = glm::vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]);
        frustum.planes[5].distance = m[3][3] - m[3][2];

        // Normalize the planes
        for (int i = 0; i < 6; ++i) {
            float length = glm::length(frustum.planes[i].normal);
            frustum.planes[i].normal /= length;
            frustum.planes[i].distance /= length;
        }

        return frustum;
    }
}
