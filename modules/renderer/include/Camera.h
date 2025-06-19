#pragma once

#include <glm/glm.hpp>

namespace RDE {
    class Camera {
    public:
        Camera() = default;

        // Make the destructor virtual for a polymorphic base class
        virtual ~Camera() = default;

        const glm::mat4 &get_projection_matrix() const { return m_projection_matrix; }

    protected:
        glm::mat4 m_projection_matrix{1.0f};
    };
}