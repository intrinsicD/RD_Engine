#pragma once

#include "geometry/Plane.h"

#include <glm/glm.hpp>
#include <variant>

namespace RDE {
    struct CameraMatrices {
        glm::mat4 view_matrix = glm::mat4(1.0f); // View matrix
        glm::mat4 projection_matrix = glm::mat4(1.0f); // Projection matrix
    };

    struct CameraViewParameters {
        glm::vec3 position; // Camera position in world space
        glm::vec3 forward; // Forward direction vector
        glm::vec3 up; // Up direction vector

        glm::vec3 get_right() const {
            return glm::normalize(glm::cross(forward, up)); // Right direction vector
        }
    };

    struct CameraProjectionParameters {
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

    struct CameraFrustumPlanes {
        Plane planes[6]; // Six planes defining the frustum (left, right, top, bottom, near, far)
    };

    struct CameraDirty {
    };

    struct CameraPrimary {
    };

    struct CameraComponent{
        CameraProjectionParameters projection_params; // Projection parameters of the camera
    };
}

#include <entt/fwd.hpp>

namespace RDE::CameraUtils{
    glm::mat4 CalculateViewMatrixFromModelMatrix(const glm::mat4 &model_matrix);

    CameraViewParameters GetViewParamsFromViewMatrix(const glm::mat4 &view_matrix);

    glm::mat4 CalculatePerspectiveProjectionMatrix(const CameraProjectionParameters::Perspective &perspective_params,
                                                   float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateOrthographicProjectionMatrix(const CameraProjectionParameters::Orthographic &orthographic_params,
                                                    float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateProjectionMatrix(const CameraProjectionParameters &projection_params);

    CameraFrustumPlanes CalculateFrustumPlanes(const glm::mat4 &view_projection_matrix);

    entt::entity CreateCameraEntity(entt::registry &registry);

    entt::entity CreateCameraEntity(entt::registry &registry, entt::entity entity_id);

    bool MakeCameraEntityPrimary(entt::registry &registry, entt::entity entity_id);

    entt::entity GetCameraEntityPrimary(entt::registry &registry);

    void SetCameraDirty(entt::registry &registry, entt::entity entity_id);
}