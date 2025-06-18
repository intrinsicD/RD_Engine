// RDE_Project/modules/renderer/include/Renderer/OrthographicCameraController.h
#pragma once

#include "Renderer/OrthographicCamera.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/MouseEvent.h"

// Note: In a real engine, we'd have a Timestep class/value passed to OnUpdate.
// For now, we'll use a hardcoded speed.

class OrthographicCameraController {
public:
    OrthographicCameraController(float aspectRatio, bool rotation = false); // aspect ratio = width / height

    void OnUpdate(float ts); // ts = timestep, e.g., time in seconds since last frame
    void OnEvent(Event &e);

    OrthographicCamera &GetCamera() { return m_camera; }

    const OrthographicCamera &GetCamera() const { return m_camera; }

private:
    bool OnMouseScrolled(MouseScrolledEvent &e);

    bool OnWindowResized(WindowResizeEvent &e);

private:
    float m_aspect_ratio;
    float m_zoom_level = 1.0f;
    OrthographicCamera m_camera;

    bool m_rotation_enabled;
    // We will add rotation variables later if needed

    glm::vec3 m_camera_position = {0.0f, 0.0f, 0.0f};
    float m_camera_move_speed = 5.0f;
};