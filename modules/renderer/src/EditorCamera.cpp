#include "EditorCamera.h"
#include "Input.h"
#include "Base.h"

#include <GLFW/glfw3.h> // For key codes

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

namespace RDE {
    EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip)
            : PerspectiveCamera(fov, aspect_ratio, near_clip, far_clip) {
        update_view_matrix();
    }

    void EditorCamera::on_update(float delta_time) {
        const glm::vec2 &mouse{Input::GetMouseX(), Input::GetMouseY()};
        glm::vec2 delta = (mouse - m_initial_mouse_position) * 0.003f;
        m_initial_mouse_position = mouse;

        if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
            mouse_pan(delta);
        else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
            mouse_rotate(delta);

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

    void EditorCamera::mouse_rotate(const glm::vec2 &delta) {
        float yaw_sign = get_up_direction().y < 0 ? -1.0f : 1.0f;
        m_yaw += yaw_sign * delta.x * 2.0f;
        m_pitch += delta.y * 2.0f;
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

    glm::quat EditorCamera::get_orientation() const { return glm::quat(glm::vec3(-m_pitch, -m_yaw, 0.0f)); }

    void EditorCamera::update_view_matrix() {
        m_position = calculate_position();
        m_orientation = get_orientation();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_orientation);
        m_view_matrix = glm::inverse(transform);
    }
}