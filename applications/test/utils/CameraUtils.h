#pragma once

#include "../components/CameraComponent.h"

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