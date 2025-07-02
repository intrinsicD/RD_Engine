#pragma once

#include "Ray.h"
#include "Plane.h"

#include <glm/glm.hpp>
#include <variant>

namespace RDE {
    struct CameraMatrices {
        glm::mat4 view_matrix = glm::mat4(1.0f); // View matrix
        glm::mat4 projection_matrix = glm::mat4(1.0f);; // Projection matrix
    };

    struct CameraViewParameters {
        glm::vec3 position; // Camera position in world space
        glm::vec3 forward; // Forward direction vector
        glm::vec3 up; // Up direction vector

        glm::vec3 get_right() const {
            return glm::normalize(glm::cross(forward, up)); // Right direction vector
        }
    };

    struct CameraPerspectiveParameters {
        float fov_degrees = 45; // Field of view in degrees
        float aspect_ratio; // Aspect ratio (width / height)

        void set_aspect_ratio(float width, float height) {
            aspect_ratio = width / height; // Set aspect ratio based on width and height
        }
    };

    struct CameraOrthographicParameters {
        float left; // Left clipping plane
        float right; // Right clipping plane
        float bottom; // Bottom clipping plane
        float top; // Top clipping plane
    };

    struct CameraProjectionParameters {
        std::variant<CameraPerspectiveParameters, CameraOrthographicParameters> projection_params; // Projection parameters
        float near_plane; // Near clipping plane distance
        float far_plane; // Far clipping plane distance
    };

    struct FrustumPlanes {
        Plane planes[6]; // Six planes defining the frustum (left, right, top, bottom, near, far)
    };

    struct DirtyCamera {
    };
    struct PrimaryCamera {
    };

    glm::mat4 CalculateViewMatrix(const CameraViewParameters &view_params);

    glm::mat4 CalculatePerspectiveProjectionMatrix(const CameraPerspectiveParameters &perspective_params,
                                                   float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateOrthographicProjectionMatrix(const CameraOrthographicParameters &orthographic_params,
                                                    float near_plane = 0.1f, float far_plane = 100.0f);

    glm::mat4 CalculateProjectionMatrix(const CameraProjectionParameters &projection_params);

    FrustumPlanes CalculateFrustumPlanes(const CameraMatrices &camera_matrices);

    class CameraViewController {
    public:
        CameraViewController(CameraViewParameters &view_params) : m_view_params(view_params) {}

        void set_position(const glm::vec3 &position);

        void set_forward(const glm::vec3 &forward);

        void set_up(const glm::vec3 &up);

        void translate(const glm::vec3 &translation);

        void rotate(const glm::vec3 &axis, float angle_degrees);

        void rotate(const glm::quat &q);

        void rotate(const glm::mat3 &rot_matrix);

        void focus_on(const glm::vec3 &target, float distance);

    private:
        CameraViewParameters &m_view_params; // Reference to the camera view parameters
    };

    class CameraZoomController {
    public:
        CameraZoomController(CameraProjectionParameters &projection_params) : m_projection_params(projection_params) {}

        void zoom(float delta);

    private:
        CameraProjectionParameters &m_projection_params; // Reference to the camera projection parameters
    };

    class CameraArcBallController {
    public:
        CameraArcBallController(CameraViewParameters &view_params, const glm::vec3 &target_world_space) : m_view_params(
                view_params), m_target_world_space(target_world_space) {

        }

        glm::vec3 get_target_world_space() const {
            return m_target_world_space;
        }

        void rotate(const glm::vec2 &screen_space_point, int screen_width, int screen_height);

        void rotate_around_target(const glm::vec3 &target_world_space, const glm::quat &quat);

        void rotate_around_target(const glm::vec3 &target_world_space, const glm::mat3 &rot_matrix);

        void rotate_around_target(const glm::vec3 &target_world_space, const glm::vec3 &axis, float angle);

    private:
        bool map_to_sphere(const glm::vec2 &screen_space_point, int screen_width, int screen_height,
                           glm::vec3 &result_on_sphere) const;

        CameraViewParameters &m_view_params; // Reference to the camera view parameters
        glm::vec3 m_target_world_space; // Target point the camera is looking at
        bool m_last_point_ok = false; // Flag to check if the last point is valid
        glm::vec3 m_last_point_3d; // Last point in 3D space
        glm::vec2 m_last_point_2d; // Last point in 2D screen space
    };

    class CameraFirstPersonController {
    public:
        CameraFirstPersonController(CameraViewParameters &view_params) : m_view_params(view_params) {}

        void move_forward(float distance);

        void move_backward(float distance);

        void strafe_left(float distance);

        void strafe_right(float distance);

        void look_around(float delta_x, float delta_y);

    private:
        CameraViewParameters &m_view_params; // Reference to the camera view parameters
    };

    class CameraPixelPerfectDragController {
    public:
        CameraPixelPerfectDragController(CameraViewParameters &view_params,
                                         const CameraProjectionParameters &projection_params)
                : m_view_params(view_params), m_projection_params(projection_params) {}

        // Call this when the drag starts (e.g., on a mouse down event)
        void start_drag(const glm::vec2 &start_mouse_position, const glm::vec3 &drag_point_world_space);

        // Call this during the drag (e.g., on a mouse move event)
        void drag(const glm::vec2 &current_mouse_position, int sceen_width, int screen_height);

        // Call this when the drag ends (e.g., on a mouse up event)
        void end_drag();

    private:
        // Helper to unproject screen coordinates to a world space ray
        Ray unproject(const glm::vec2 &screen_coords, int screen_width, int screen_height) const;

        // Helper to find the intersection of a ray and a plane
        bool ray_plane_intersection(const Ray &ray, const Plane &plane, float &out_distance) const;

        CameraViewParameters &m_view_params;
        const CameraProjectionParameters &m_projection_params;
        glm::vec2 m_start_mouse_position;
        glm::vec3 m_initial_camera_position;
        glm::vec3 m_drag_plane_point;
        bool m_is_dragging = false;
    };
}