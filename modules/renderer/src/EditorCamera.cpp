#include "EditorCamera.h"
#include "Input.h"
#include "Base.h"

#include <GLFW/glfw3.h> // For key codes

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

namespace RDE {
    EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip)
            : PerspectiveCamera(fov, aspect_ratio, near_clip, far_clip) {

        m_rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
        update_view_matrix();
    }

    void EditorCamera::on_update(float delta_time) {
        const glm::vec2 &current_mouse = {Input::GetMouseX(), Input::GetMouseY()};
        glm::vec2 delta = current_mouse - m_last_mouse_position;

        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
            mouse_pan(delta);
        else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            mouse_rotate(current_mouse); // Pass current pos and delta
        }

        m_last_mouse_position = current_mouse;
        m_last_point_ok = map_to_sphere(current_mouse, m_last_mouse_position_3d);
        update_view_matrix();
    }

    void EditorCamera::on_event(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::on_mouse_scroll));
    }

    bool EditorCamera::on_mouse_scroll(MouseScrolledEvent &e) {
        float delta = e.GetYOffset() * 0.1f;
        mouse_zoom(delta);
        return true;
    }

    void EditorCamera::mouse_pan(const glm::vec2 &delta) {
        m_focal_point += -get_right_direction() * delta.x * m_distance * 0.5f;
        m_focal_point += get_up_direction() * delta.y * m_distance * 0.5f;
    }

    void EditorCamera::mouse_rotate(const glm::vec2 &current_pos) {
        if(m_last_point_ok){
            glm::vec3 result;
            bool new_point_ok = map_to_sphere(current_pos, result);

            if (new_point_ok) {
                float cos_angle = glm::dot(result, m_last_mouse_position_3d); // Cosine of the angle between the two vectors in radians
                cos_angle = glm::clamp(cos_angle, -1.0f, 1.0f);

                if (fabs(cos_angle) < 1.0f) {
                    float angle_radians = acos(cos_angle);
                    // The rotation axis is in screen space. Convert to world space.
                    glm::vec3 axis_view_space = glm::normalize(cross(m_last_mouse_position_3d, result)); // Axis of rotation in world space? its in screen space
                    glm::vec3 axis_world_space = glm::normalize(glm::inverse(get_orientation()) * axis_view_space);

                    // Apply the delta rotation to our primary state variable.
                    m_rotation =  glm::angleAxis(-2.0f * angle_radians, axis_world_space) * m_rotation;
                    //TODO add debug info about the rotation when it breaks down
                }
            }
        }
    }

    bool EditorCamera::map_to_sphere(const glm::vec2 &point, glm::vec3 &result) const {
        if ((point.x >= 0) && (point.x <= m_viewport_width) && (point.y >= 0) && (point.y <= m_viewport_height)) {
            double x = (double) (point.x - 0.5 * m_viewport_width) / m_viewport_width;
            double y = (double) (0.5 * m_viewport_height - point.y) / m_viewport_height;
            double sinx = sin(std::numbers::pi * x * 0.5);
            double siny = sin(std::numbers::pi * y * 0.5);
            double sinx2siny2 = sinx * sinx + siny * siny;

            result[0] = sinx;
            result[1] = siny;
            result[2] = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;
            return true;
        }
        return false;
    }

    void EditorCamera::mouse_zoom(float delta) {
        m_distance -= delta * 5.0f;
        if (m_distance < 1.0f)
            m_distance = 1.0f;
    }

    glm::vec3 EditorCamera::get_up_direction() const {
        return glm::rotate(get_orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::get_right_direction() const {
        return glm::rotate(get_orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::get_forward_direction() const {
        return glm::rotate(get_orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 EditorCamera::calculate_position() const { return m_focal_point - get_forward_direction() * m_distance; }

    glm::quat EditorCamera::get_orientation() const { return m_rotation; }

    void EditorCamera::update_view_matrix() {
        m_position = calculate_position();
        m_view_matrix = glm::lookAt(m_position, m_focal_point, get_up_direction());
    }
}