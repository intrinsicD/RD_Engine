#pragma once

#include <glm/glm.hpp>

namespace RDE {
    struct OrthographicComponent {
        float left = -1.0f; // Left clipping plane
        float right = 1.0f; // Right clipping plane
        float bottom = -1.0f; // Bottom clipping plane
        float top = 1.0f; // Top clipping plane
        float zNear = 0.1f; // Near clipping plane
        float zFar = 100.0f; // Far clipping plane
    };
}