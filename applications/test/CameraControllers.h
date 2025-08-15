#pragma once

#include "components/CameraComponent.h" // fixed include
#include "geometry/Ray.h"

namespace RDE::Camera {
    // Renamed aliases for clarity with existing component types
    using ViewParameters = CameraViewParameters;
    using ProjectionParameters = CameraProjectionParameters;

    class ViewController {
    public:
        explicit ViewController(ViewParameters &view_params) : m_view_params(view_params) {}

        void set_position(const glm::vec3 &position);

        void set_forward(const glm::vec3 &forward);

        void set_up(const glm::vec3 &up);

        void translate(const glm::vec3 &translation);

        void rotate(const glm::vec3 &axis, float angle_degrees);

        void rotate(const glm::quat &q);

        void rotate(const glm::mat3 &rot_matrix);

        void focus_on(const glm::vec3 &target, float distance);

    private:
        ViewParameters &m_view_params; // Reference to the camera view parameters
    };

    class ZoomController {
    public:
        explicit ZoomController(ProjectionParameters &projection_params) : m_projection_params(projection_params) {}

        void zoom(float delta);

    private:
        ProjectionParameters &m_projection_params; // Reference to the camera projection parameters
    };

    class ArcBallController {
    public:
        ArcBallController(ViewParameters &view_params, const glm::vec3 &target_world_space) : m_view_params(
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

        ViewParameters &m_view_params; // Reference to the camera view parameters
        glm::vec3 m_target_world_space{}; // Target point the camera is looking at
        bool m_last_point_ok = false; // Flag to check if the last point is valid
        glm::vec3 m_last_point_3d{}; // Last point in 3D space
        glm::vec2 m_last_point_2d{}; // Last point in 2D screen space
    };

    class FirstPersonController {
    public:
        explicit FirstPersonController(ViewParameters &view_params) : m_view_params(view_params) {}

        void move_forward(float distance);

        void move_backward(float distance);

        void strafe_left(float distance);

        void strafe_right(float distance);

        void look_around(float delta_x, float delta_y);

    private:
        ViewParameters &m_view_params; // Reference to the camera view parameters
    };

    class PixelPerfectDragController {
    public:
        PixelPerfectDragController(ViewParameters &view_params,
                                         const ProjectionParameters &projection_params)
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

        ViewParameters &m_view_params;
        const ProjectionParameters &m_projection_params;
        glm::vec2 m_start_mouse_position{};
        glm::vec3 m_initial_camera_position{};
        glm::vec3 m_drag_plane_point{};
        bool m_is_dragging = false;
    };

    class TrackballController {
    public:
        explicit TrackballController(ViewParameters &view_params,
                            const glm::vec3 &scene_center = glm::vec3(0.0f),
                            float scene_radius = 1.0f)
                : m_view_params(view_params),
                  m_scene_center(scene_center),
                  m_scene_radius(scene_radius) {}

        void set_scene(const glm::vec3 &center, float radius) { m_scene_center = center; m_scene_radius = radius; }

        // Begin a potential rotation (mouse down)
        void begin_rotate(const glm::vec2 &screen_point, int width, int height);
        // Update rotation (mouse move while rotating)
        void update_rotate(const glm::vec2 &screen_point, int width, int height);
        void end_rotate();

        // Pan (translate perpendicular to view direction)
        void pan(float dx_pixels, float dy_pixels);
        // Dolly/zoom (positive delta > zoom in)
        void dolly(float scroll_delta);

        // Reframe entire scene
        void view_all();

    private:
        bool map_to_sphere(const glm::vec2 &p, int w, int h, glm::vec3 &out) const;
        void apply_rotation(const glm::vec3 &from, const glm::vec3 &to);

        ViewParameters &m_view_params;
        glm::vec3 m_scene_center{0.0f};
        float m_scene_radius{1.0f};
        glm::vec2 m_prev_point_2d{};
        glm::vec3 m_prev_point_3d{};
        bool m_prev_ok = false;
        bool m_rotating = false;
    };
}