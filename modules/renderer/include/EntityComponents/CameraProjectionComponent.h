// file: EntityComponents/CameraProjection.h
#pragma once

#include <glm/glm.hpp>
#include <variant>

namespace RDE {
    // Pure data for a perspective projection
    struct PerspectiveProjection {
        float fovy_degrees = 45.0f;
        float aspect_ratio = 1.0f;
        float zNear = 0.1f;
        float zFar = 1000.0f;
    };

    // Pure data for an orthographic projection
    struct OrthographicProjection {
        float left = -1.0f;
        float right = 1.0f;
        float bottom = -1.0f;
        float top = 1.0f;
        float zNear = -1.0f;
        float zFar = 1.0f;
    };

    // This component defines the projection properties of a camera.
    // It can hold either a perspective or an orthographic projection configuration.
    struct CameraProjectionComponent {
        std::variant<PerspectiveProjection, OrthographicProjection> projection_data;
    };
}
