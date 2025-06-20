// RDE_Project/modules/renderer/src/OrthographicCameraController.cpp
#include "OrthographicCameraController.h"
#include "Input.h"
#include "Base.h"

#include <functional>

namespace RDE {
    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
            : m_aspect_ratio(aspectRatio),
              m_camera(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level, m_zoom_level),
              m_rotation_enabled(rotation) {
    }

    void OrthographicCameraController::on_update(float ts) {
        // For now, we'll poll for keyboard state. A better system might use key events.
        if (Input::IsKeyPressed(87)) // W key
            m_camera_position.y += m_camera_move_speed * ts;
        else if (Input::IsKeyPressed(83)) // S key
            m_camera_position.y -= m_camera_move_speed * ts;

        if (Input::IsKeyPressed(65)) // A key
            m_camera_position.x -= m_camera_move_speed * ts;
        else if (Input::IsKeyPressed(68)) // D key
            m_camera_position.x += m_camera_move_speed * ts;

  /*      m_camera.SetPosition(m_camera_position);*/
    }

    void OrthographicCameraController::on_event(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::on_mouse_scrolled));
        dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCameraController::on_window_resized));
    }

    bool OrthographicCameraController::on_mouse_scrolled(MouseScrolledEvent &e) {
        m_zoom_level -= e.get_y_offset() * 0.25f;
        m_zoom_level = std::max(m_zoom_level, 0.25f);
/*        m_camera.SetProjection(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level,
                               m_zoom_level);*/
        return false; // Allow other layers to process the event
    }

    bool OrthographicCameraController::on_window_resized(WindowResizeEvent &e) {
        m_aspect_ratio = (float) e.get_width() / (float) e.get_height();
/*        m_camera.SetProjection(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level,
                               m_zoom_level);*/
        return false;
    }
}