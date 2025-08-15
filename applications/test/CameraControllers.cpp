#include "CameraControllers.h"
#include "components/CameraComponent.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

namespace RDE::Camera {
    void ViewController::set_position(const glm::vec3 &position) {
        m_view_params.position = position;
    }

    void ViewController::set_forward(const glm::vec3 &forward) {
        m_view_params.forward = glm::normalize(forward);
        // Ensure the up vector is orthogonal to the forward vector
        m_view_params.up = glm::normalize(glm::cross(m_view_params.forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void ViewController::set_up(const glm::vec3 &up) {
        m_view_params.up = glm::normalize(up);
        // Ensure the forward vector is orthogonal to the up vector
        m_view_params.forward = glm::normalize(glm::cross(m_view_params.up, glm::vec3(0.0f, 0.0f, -1.0f)));
    }

    void ViewController::translate(const glm::vec3 &translation) {
        m_view_params.position += translation;
    }

    void ViewController::rotate(const glm::vec3 &axis, float angle_degrees) {
        glm::mat3 rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle_degrees), axis);
        rotate(rotation_matrix);
    }

    void ViewController::rotate(const glm::quat &q) {
        // Rotate the forward and up vectors using quaternion multiplication
        m_view_params.forward = glm::normalize(q * m_view_params.forward);
        m_view_params.up = glm::normalize(q * m_view_params.up);
    }

    void ViewController::rotate(const glm::mat3 &rot_matrix) {
        // Rotate the forward and up vectors
        m_view_params.forward = glm::normalize(rot_matrix * m_view_params.forward);
        m_view_params.up = glm::normalize(rot_matrix * m_view_params.up);
    }

    void ViewController::focus_on(const glm::vec3 &target, float distance) {
        // Set the camera position to be at a distance from the target
        m_view_params.position = target - m_view_params.forward * distance;
        // Ensure the camera is looking at the target
        m_view_params.forward = glm::normalize(target - m_view_params.position);
    }

    void ZoomController::zoom(float delta) {
        // Adjust the field of view based on zoom input
        if (std::holds_alternative<ProjectionParameters::Perspective>(m_projection_params.parameters)) {
            auto &perspective_params = std::get<ProjectionParameters::Perspective>(m_projection_params.parameters);
            perspective_params.fov_degrees -= delta;
            perspective_params.fov_degrees = glm::clamp(perspective_params.fov_degrees, 1.0f, 45.0f); // Clamp to reasonable bounds
        } else if (std::holds_alternative<ProjectionParameters::Orthographic>(m_projection_params.parameters)) {
            auto &orthographic_params = std::get<ProjectionParameters::Orthographic>(m_projection_params.parameters);
            orthographic_params.left += delta;
            orthographic_params.right -= delta;
            orthographic_params.bottom += delta;
            orthographic_params.top -= delta;
        }
    }

    bool ArcBallController::map_to_sphere(const glm::vec2 &screen_space_point, int screen_width, int screen_height,
                                           glm::vec3 &result_on_sphere) const {
        //Maps a 2D screen point to a 3D point on a virtual sphere using Shoemake's sinusoidal projection.
        if (screen_space_point.x < 0.0f || screen_space_point.x > screen_width ||
            screen_space_point.y < 0.0f || screen_space_point.y > screen_height) {
            return false;
        }

        const float mapped_x = (screen_space_point.x - 0.5f * screen_width) / screen_width;
        const float mapped_y = (0.5f * screen_height - screen_space_point.y) / screen_height;
        const float sinx = std::sin(std::numbers::pi_v<float> * mapped_x * 0.5);
        const float siny = std::sin(std::numbers::pi_v<float> * mapped_y * 0.5);
        const float sinx2siny2 = (sinx * sinx) + (siny * siny);
        const float z = sinx2siny2 < 1.0f ? std::sqrt(1.0f - sinx2siny2) : 0.0f;
        result_on_sphere = glm::vec3(sinx, siny, z);
        return true;
    }

    void ArcBallController::rotate(const glm::vec2 &screen_space_point, int screen_width, int screen_height) {
        if (m_last_point_ok) {
            glm::vec3 result_on_sphere;
            bool new_point_ok = map_to_sphere(screen_space_point, screen_width, screen_height, result_on_sphere);

            if (new_point_ok) {
                glm::vec3 axis = cross(m_last_point_3d, result_on_sphere);
                float cos_angle = glm::dot(m_last_point_3d, result_on_sphere);

                if (fabs(cos_angle) < 1.0) {
                    float angle_degrees = glm::degrees(acos(cos_angle));
                    rotate_around_target(m_target_world_space, axis, -angle_degrees);
                }
            }
        }
        // Update the last point in 3D space
        m_last_point_2d = screen_space_point;
        m_last_point_ok = map_to_sphere(m_last_point_2d, screen_width, screen_height, m_last_point_3d);
    }

    void ArcBallController::rotate_around_target(const glm::vec3 &target_world_space, const glm::quat &quat) {
        glm::vec3 direction = m_view_params.position - target_world_space;
        glm::vec3 rotated_direction = quat * direction;
        m_view_params.position = target_world_space + rotated_direction;
        m_view_params.up = glm::normalize(quat * m_view_params.up);
        m_view_params.forward = glm::normalize(target_world_space - m_view_params.position);
    }

    void ArcBallController::rotate_around_target(const glm::vec3 &target_world_space, const glm::mat3 &rot_matrix) {
        // Rotate the camera view parameters around the target point
        m_view_params.position = target_world_space + rot_matrix * (m_view_params.position - target_world_space);
        m_view_params.forward = glm::normalize(target_world_space - m_view_params.position);
        m_view_params.up = glm::normalize(rot_matrix * m_view_params.up);
    }

    void FirstPersonController::move_forward(float distance) {
        glm::vec3 forward = glm::normalize(m_view_params.forward);
        m_view_params.position += forward * distance;
    }

    void FirstPersonController::move_backward(float distance) {
        glm::vec3 backward = glm::normalize(-m_view_params.forward);
        m_view_params.position += backward * distance;
    }

    void FirstPersonController::strafe_left(float distance) {
        glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
        m_view_params.position -= right * distance;
    }

    void FirstPersonController::strafe_right(float distance) {
        glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
        m_view_params.position += right * distance;
    }

    void FirstPersonController::look_around(float delta_x, float delta_y) {
        // Adjust the forward vector based on mouse movement
        float sensitivity = 0.1f; // Adjust sensitivity as needed
        glm::quat yaw = glm::angleAxis(glm::radians(delta_x * sensitivity), m_view_params.up);
        glm::quat pitch = glm::angleAxis(glm::radians(delta_y * sensitivity), glm::normalize(glm::cross(m_view_params.forward, m_view_params.up)));

        // Apply the rotations to the forward vector
        m_view_params.forward = glm::normalize(yaw * pitch * m_view_params.forward);
        m_view_params.up = glm::normalize(glm::cross(m_view_params.forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void ArcBallController::rotate_around_target(const glm::vec3 &target_world_space, const glm::vec3 &axis,
                                                       float angle) {
        glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
        glm::vec3 direction = m_view_params.position - target_world_space;
        glm::vec3 rotated_direction = glm::vec3(rotation_matrix * glm::vec4(direction, 1.0f));
        m_view_params.position = target_world_space + rotated_direction;
        m_view_params.forward = glm::normalize(target_world_space - m_view_params.position);
        m_view_params.up = glm::normalize(glm::cross(m_view_params.forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    void PixelPerfectDragController::start_drag(const glm::vec2 &start_mouse_position,
                                                      const glm::vec3 &drag_point_world_space) {
        m_start_mouse_position = start_mouse_position;
        m_initial_camera_position = m_view_params.position;
        m_drag_plane_point = drag_point_world_space;
        m_is_dragging = true;
    }

    void PixelPerfectDragController::drag(const glm::vec2 &current_mouse_position,
                                                int sceen_width, int screen_height) {
        if (!m_is_dragging) {
            return;
        }

        // Define the drag plane: normal is the camera's forward vector, and it passes through m_drag_plane_point
        glm::vec3 drag_plane_normal = -m_view_params.forward;
        Plane drag_plane = {drag_plane_normal, glm::dot(drag_plane_normal, m_drag_plane_point)};

        // Get the ray for the current mouse position
        Ray current_ray = unproject(current_mouse_position, sceen_width, screen_height);

        // Find the intersection of the current ray with the drag plane
        float intersection_distance;
        if (ray_plane_intersection(current_ray, drag_plane, intersection_distance)) {
            glm::vec3 current_world_position = current_ray.origin + current_ray.direction * intersection_distance;

            // The vector from the initially clicked point to the new point on the plane
            glm::vec3 drag_delta = current_world_position - m_drag_plane_point;

            // The new camera position is the initial camera position minus the drag delta
            // We subtract because if you drag the world to the left, the camera moves to the right
            m_view_params.position = m_initial_camera_position - drag_delta;
        }
    }

    void PixelPerfectDragController::end_drag() {
        m_is_dragging = false;
    }

    Ray PixelPerfectDragController::unproject(const glm::vec2 &screen_coords, int screen_width,
                                                    int screen_height) const {
        //TODO move to a utility file

        // Convert screen coordinates to normalized device coordinates (NDC)
        float x = (2.0f * screen_coords.x) / screen_width - 1.0f;
        float y = 1.0f - (2.0f * screen_coords.y) / screen_height;
        float z = 1.0f; // Furthest point in the clip space cube
        glm::vec3 ray_nds = glm::vec3(x, y, z);

        // Convert to clip space
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // Convert to eye space
        glm::mat4 inv_projection = glm::inverse(CameraUtils::CalculateProjectionMatrix(m_projection_params));
        glm::vec4 ray_eye = inv_projection * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Convert to world space
        glm::mat4 inv_view = glm::inverse(CameraUtils::CalculateViewMatrix(m_view_params));
        glm::vec3 ray_world_dir = glm::normalize(glm::vec3(inv_view * ray_eye));

        return {m_view_params.position, ray_world_dir};
    }

    bool PixelPerfectDragController::ray_plane_intersection(const Ray &ray,
                                                                  const Plane &plane,
                                                                  float &out_distance) const {
        //TODO move to a utility file
        float denominator = glm::dot(plane.normal, ray.direction);
        if (std::abs(denominator) > 1e-6) { // Avoid division by zero
            float numerator = glm::dot(plane.normal, plane.normal * plane.distance - ray.origin);
            out_distance = numerator / denominator;
            return out_distance >= 0; // Intersection must be in front of the ray's origin
        }
        return false;
    }

    bool TrackballController::map_to_sphere(const glm::vec2 &p, int w, int h, glm::vec3 &out) const {
        if (p.x < 0.f || p.x > (float)w || p.y < 0.f || p.y > (float)h) return false;
        float x = (p.x - 0.5f * w) / (float)w;            // [-0.5,0.5]
        float y = (0.5f * h - p.y) / (float)h;             // [-0.5,0.5] with y up
        float sinx = std::sin(std::numbers::pi_v<float> * x);
        float siny = std::sin(std::numbers::pi_v<float> * y);
        float ss = sinx * sinx + siny * siny;
        float z = ss < 1.f ? std::sqrt(1.f - ss) : 0.f;
        out = glm::vec3(sinx, siny, z);
        return true;
    }

    void TrackballController::apply_rotation(const glm::vec3 &from, const glm::vec3 &to) {
        glm::vec3 axis = glm::cross(from, to);
        float dotv = glm::clamp(glm::dot(from, to), -1.f, 1.f);
        if (glm::length(axis) < 1e-8f || fabsf(dotv) > 0.9999f) return; // negligible
        float angle = std::acos(dotv); // radians
        // Rotate camera position around scene center
        glm::vec3 cam_dir = m_view_params.position - m_scene_center;
        glm::mat4 R = glm::rotate(glm::mat4(1.f), angle, glm::normalize(axis));
        cam_dir = glm::vec3(R * glm::vec4(cam_dir, 1.f));
        m_view_params.position = m_scene_center + cam_dir;
        // Rotate up vector
        m_view_params.up = glm::normalize(glm::vec3(R * glm::vec4(m_view_params.up, 0.f)));
        // Recompute forward
        m_view_params.forward = glm::normalize(m_scene_center - m_view_params.position);
    }

    void TrackballController::begin_rotate(const glm::vec2 &screen_point, int width, int height) {
        m_rotating = true;
        m_prev_ok = map_to_sphere(screen_point, width, height, m_prev_point_3d);
        m_prev_point_2d = screen_point;
    }

    void TrackballController::update_rotate(const glm::vec2 &screen_point, int width, int height) {
        if (!m_rotating || !m_prev_ok) return;
        glm::vec3 cur3d;
        if (map_to_sphere(screen_point, width, height, cur3d)) {
            apply_rotation(m_prev_point_3d, cur3d);
            m_prev_point_3d = cur3d;
            m_prev_point_2d = screen_point;
        }
    }

    void TrackballController::end_rotate() {
        m_rotating = false;
        m_prev_ok = false;
    }

    void TrackballController::pan(float dx_pixels, float dy_pixels) {
        // Determine scale factor based on distance & FOV
        float distance = glm::length(m_view_params.position - m_scene_center);
        glm::vec3 right = glm::normalize(glm::cross(m_view_params.forward, m_view_params.up));
        float pixels_per_unit = 800.f; // heuristic reference scale
        float scale = (distance * 2.f) / pixels_per_unit; // more distance => larger pan per pixel
        glm::vec3 translation = (-right * dx_pixels + m_view_params.up * dy_pixels) * scale;
        m_view_params.position += translation;
        m_scene_center += translation; // keep orbit target consistent
    }

    void TrackballController::dolly(float scroll_delta) {
        // Move camera toward/away from center exponentially for stability
        float distance = glm::length(m_view_params.position - m_scene_center);
        distance *= std::pow(0.9f, scroll_delta); // scroll_delta positive zooms in
        distance = glm::clamp(distance, 0.05f * m_scene_radius, 50.f * m_scene_radius);
        m_view_params.position = m_scene_center - m_view_params.forward * distance;
    }

    void TrackballController::view_all() {
        // Place camera back so entire scene fits (~2.5 * radius like reference)
        if (glm::length(m_view_params.forward) < 1e-6f) m_view_params.forward = glm::vec3(0.f, 0.f, -1.f);
        if (glm::length(m_view_params.up) < 1e-6f) m_view_params.up = glm::vec3(0.f, 1.f, 0.f);
        m_view_params.position = m_scene_center - m_view_params.forward * (2.5f * m_scene_radius);
    }
}
