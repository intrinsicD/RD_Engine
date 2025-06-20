#pragma once

#include <glm/glm.hpp>

namespace RDE {
    struct PerspectiveComponent {
        float fovy_degrees = 45.0f; // Field of view in degrees
        float aspect_ratio = 1.0f; // Aspect ratio
        float zNear = 0.1f; // Near clipping plane
        float zFar = 100.0f; // Far clipping plane
    };
}