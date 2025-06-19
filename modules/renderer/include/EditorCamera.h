#pragma once

#include "PerspectiveCamera.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"

#include <glm/gtc/quaternion.hpp>

namespace RDE {
    class EditorCamera : public PerspectiveCamera {
    public:
        EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip);

        void on_update(float delta_time);

        void on_event(Event &e);

        const glm::mat4 &get_view_matrix() const { return m_view_matrix; }

        glm::mat4 get_view_projection() const { return m_projection_matrix * m_view_matrix; }

        float get_distance() const { return m_distance; }

        void set_distance(float distance) { m_distance = distance; }

        const glm::vec3 &get_focal_point() const { return m_focal_point; }

        void set_viewport_size(float width, float height){
            m_viewport_width = width;
            m_viewport_height = height;
            set_aspect_ratio(width / height);
            update_view_matrix();
        }

    private:
        bool on_mouse_scroll(MouseScrolledEvent &e);

        void mouse_pan(const glm::vec2 &delta);

        void mouse_rotate(const glm::vec2& current_pos);

        bool map_to_sphere(const glm::vec2 &point, glm::vec3 &result) const;

        void mouse_zoom(float delta);

        glm::vec3 get_up_direction() const;

        glm::vec3 get_right_direction() const;

        glm::vec3 get_forward_direction() const;

        glm::vec3 calculate_position() const;

        glm::quat get_orientation() const;

        void update_view_matrix();

    private:
        glm::mat4 m_view_matrix{1.0f};
        glm::vec3 m_position{0.0f, 0.0f, 0.0f};
        glm::vec3 m_focal_point{0.0f, 0.0f, 0.0f};
        glm::quat m_rotation;

        float m_distance = 10.0f;
        glm::vec2 m_last_mouse_position{0.0f};
        glm::vec3 m_last_mouse_position_3d{0.0f};
        bool m_last_point_ok = false;

        float m_viewport_width = 1280, m_viewport_height = 720;
    };
}