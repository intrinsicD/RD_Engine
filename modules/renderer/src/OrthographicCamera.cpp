// RDE_Project/modules/renderer/src/OrthographicCamera.cpp
#include "../include/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
            : m_left(left), m_right(right), m_bottom(bottom), m_top(top) {
        recalculate_projection();
    }

    void OrthographicCamera::set_projection(float left, float right, float bottom, float top) {
        m_left = left;
        m_right = right;
        m_bottom = bottom;
        m_top = top;
        recalculate_projection();
    }

    void OrthographicCamera::recalculate_projection() {
        m_projection_matrix = glm::ortho(m_left, m_right, m_bottom, m_top, -1.0f, 1.0f);
    }
}