#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    PerspectiveCamera::PerspectiveCamera(float vertical_fov, float aspect_ratio, float near_clip, float far_clip)
            : m_vertical_fov(vertical_fov), m_aspect_ratio(aspect_ratio), m_near_clip(near_clip), m_far_clip(far_clip) {
        recalculate_projection();
    }

    void PerspectiveCamera::set_projection(float vertical_fov, float aspect_ratio, float near_clip, float far_clip) {
        m_vertical_fov = vertical_fov;
        m_aspect_ratio = aspect_ratio;
        m_near_clip = near_clip;
        m_far_clip = far_clip;
        recalculate_projection();
    }

    void PerspectiveCamera::recalculate_projection() {
        m_projection_matrix = glm::perspective(glm::radians(m_vertical_fov), m_aspect_ratio, m_near_clip, m_far_clip);
    }
}