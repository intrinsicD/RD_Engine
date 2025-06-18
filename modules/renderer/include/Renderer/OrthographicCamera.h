// RDE_Project/modules/renderer/include/Renderer/OrthographicCamera.h
#pragma once

#include <glm/glm.hpp>

class OrthographicCamera {
public:
    OrthographicCamera(float left, float right, float bottom, float top);

    const glm::vec3 &GetPosition() const { return m_position; }

    void SetPosition(const glm::vec3 &position) {
        m_position = position;
        RecalculateViewMatrix();
    }

    const glm::mat4 &GetProjectionMatrix() const { return m_projection_matrix; }

    const glm::mat4 &GetViewMatrix() const { return m_view_matrix; }

    const glm::mat4 &GetViewProjectionMatrix() const { return m_view_projection_matrix; }

    void SetProjection(float left, float right, float bottom, float top);
private:
    void RecalculateViewMatrix();

    glm::mat4 m_projection_matrix;
    glm::mat4 m_view_matrix;
    glm::mat4 m_view_projection_matrix;
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
};