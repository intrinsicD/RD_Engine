// RDE_Project/modules/renderer/include/Renderer/OrthographicCameraController.h
#pragma once

#include "OrthographicCamera.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"

namespace RDE {
// Note: In a real engine, we'd have a Timestep class/value passed to OnUpdate.
// For now, we'll use a hardcoded speed.

    class OrthographicCameraController {
    public:
        OrthographicCameraController(float aspectRatio, bool rotation = false); // aspect ratio = width / height

        void on_update(float ts); // ts = timestep, e.g., time in seconds since last frame

        void on_event(Event &e);

        OrthographicCamera &get_camera() { return m_camera; }

        const OrthographicCamera &get_camera() const { return m_camera; }

    private:
        bool on_mouse_scrolled(MouseScrolledEvent &e);

        bool on_window_resized(WindowResizeEvent &e);

    private:
        float m_aspect_ratio;
        float m_zoom_level = 1.0f;
        OrthographicCamera m_camera;

        bool m_rotation_enabled;
        // We will add rotation variables later if needed

        glm::vec3 m_camera_position = {0.0f, 0.0f, 0.0f};
        float m_camera_move_speed = 5.0f;
    };
}