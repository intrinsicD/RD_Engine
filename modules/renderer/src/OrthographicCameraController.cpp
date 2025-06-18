// RDE_Project/modules/renderer/src/OrthographicCameraController.cpp
#include "Renderer/OrthographicCameraController.h"
#include "Core/Events/KeyEvent.h" // To check key presses
#include "Core/Input.h" // We will create this new input polling abstraction
namespace RDE {
    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
            : m_aspect_ratio(aspectRatio),
              m_camera(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level, m_zoom_level),
              m_rotation_enabled(rotation) {
    }

    void OrthographicCameraController::on_update(float ts) {
        // For now, we'll poll for keyboard state. A better system might use key events.
        if (Input::is_key_pressed(87)) // W key
            m_camera_position.y += m_camera_move_speed * ts;
        else if (Input::is_key_pressed(83)) // S key
            m_camera_position.y -= m_camera_move_speed * ts;

        if (Input::is_key_pressed(65)) // A key
            m_camera_position.x -= m_camera_move_speed * ts;
        else if (Input::is_key_pressed(68)) // D key
            m_camera_position.x += m_camera_move_speed * ts;

        m_camera.SetPosition(m_camera_position);
    }

    void OrthographicCameraController::OnEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(
                std::bind(&OrthographicCameraController::OnMouseScrolled, this, std::placeholders::_1));
        dispatcher.dispatch<WindowResizeEvent>(
                std::bind(&OrthographicCameraController::OnWindowResized, this, std::placeholders::_1));
    }

    bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent &e) {
        m_zoom_level -= e.GetYOffset() * 0.25f;
        m_zoom_level = std::max(m_zoom_level, 0.25f);
        m_camera.SetProjection(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level,
                               m_zoom_level);
        return false; // Allow other layers to process the event
    }

    bool OrthographicCameraController::OnWindowResized(WindowResizeEvent &e) {
        m_aspect_ratio = (float) e.GetWidth() / (float) e.GetHeight();
        m_camera.SetProjection(-m_aspect_ratio * m_zoom_level, m_aspect_ratio * m_zoom_level, -m_zoom_level,
                               m_zoom_level);
        return false;
    }
}