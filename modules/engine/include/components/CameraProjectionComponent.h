#pragma once

#include <variant>

namespace RDE::Components {
    struct CameraProjectionComponent {
        struct Perspective {
            float fov = 45.0f; // Field of view in degrees
            float aspect_ratio = 16.0f / 9.0f; // Aspect ratio
        };

        struct Orthographic {
            float left = -1.0f;
            float right = 1.0f;
            float bottom = -1.0f;
            float top = 1.0f;
        };

        float z_near = 0.1f;
        float z_far = 1000.0f;

        std::variant<Perspective, Orthographic> projection;
    };
}
