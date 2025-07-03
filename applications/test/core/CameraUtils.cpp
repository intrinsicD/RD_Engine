#include "CameraUtils.h"
#include "TransformUtils.h"

#include <entt/entity/registry.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace RDE::CameraUtils {
    glm::mat4 CalculateViewMatrixFromModelMatrix(const glm::mat4 &model_matrix) {
        return glm::inverse(model_matrix);
    }

    CameraViewParameters GetViewParamsFromViewMatrix(const glm::mat4 &view_matrix) {
        CameraViewParameters view_params;
        // Extract translation, rotation, and scale from the view matrix
        glm::vec3 translation, scale;
        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(view_matrix, scale, rotation, translation, skew, perspective);

        view_params.position = translation;
        view_params.forward = -glm::normalize(glm::vec3(view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]));
        view_params.up = glm::normalize(glm::vec3(view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]));

        return view_params;
    }

    glm::mat4 CalculateViewMatrix(const CameraViewParameters &view_params) {
        return glm::lookAt(view_params.position,
                           view_params.position + view_params.forward,
                           view_params.up);
    }

    glm::mat4 CalculatePerspectiveProjectionMatrix(const CameraProjectionParameters::Perspective &perspective_params,
                                                   float near_plane, float far_plane) {
        return glm::perspective(glm::radians(perspective_params.fov_degrees),
                                perspective_params.aspect_ratio,
                                near_plane, far_plane);
    }

    glm::mat4 CalculateOrthographicProjectionMatrix(const CameraProjectionParameters::Orthographic &orthographic_params,
                                                    float near_plane, float far_plane) {
        return glm::ortho(orthographic_params.left,
                          orthographic_params.right,
                          orthographic_params.bottom,
                          orthographic_params.top,
                          near_plane, far_plane);
    }

    glm::mat4 CalculateProjectionMatrix(const CameraProjectionParameters &params) {
        // Check if the parameters are of type Perspective or Orthographic
        if (std::holds_alternative<CameraProjectionParameters::Perspective>(params.parameters)) {
            const auto &perspective_params = std::get<CameraProjectionParameters::Perspective>(params.parameters);
            return CalculatePerspectiveProjectionMatrix(perspective_params, params.near_plane, params.far_plane);
        } else if (std::holds_alternative<CameraProjectionParameters::Orthographic>(params.parameters)) {
            const auto &orthographic_params = std::get<CameraProjectionParameters::Orthographic>(params.parameters);
            return CalculateOrthographicProjectionMatrix(orthographic_params, params.near_plane, params.far_plane);
        } else {
            return glm::mat4(1.0f); // Return identity matrix if no valid parameters are found
        }
    }

    CameraFrustumPlanes CalculateFrustumPlanes(const glm::mat4 &m) {
        CameraFrustumPlanes frustum;

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

    entt::entity CreateCameraEntity(entt::registry &registry) {
        return CreateCameraEntity(registry, entt::null);
    }

    entt::entity CreateCameraEntity(entt::registry &registry, entt::entity entity_id) {
        if (entity_id == entt::null) {
            entity_id = registry.create();
        }
        if (!registry.valid(entity_id)) return entt::null;

        registry.get_or_emplace<CameraProjectionParameters>(entity_id);
        registry.get_or_emplace<TransformLocal>(entity_id);

        return entity_id;
    }

    bool MakeCameraEntityPrimary(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<TransformLocal, CameraProjectionParameters>(entity_id)) {
            return false; // Invalid entity or not a camera
        }

        // Remove the primary camera tag from any existing primary camera
        registry.clear<CameraPrimary>();

        // Add the primary camera tag to the new camera entity
        registry.emplace<CameraPrimary>(entity_id);
        return true;
    }

    entt::entity GetCameraEntityPrimary(entt::registry &registry) {
        auto view = registry.view<CameraPrimary>();
        if (view.empty()) {
            return entt::null; // No primary camera found
        }
        // Return the first entity with the PrimaryCamera tag
        return *view.begin();
    }

    void SetCameraDirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) ||
            !registry.all_of<CameraProjectionParameters>(entity_id)) {
            return; // Invalid entity or not a camera
        }

        registry.emplace_or_replace<CameraDirty>(entity_id);
    }
}