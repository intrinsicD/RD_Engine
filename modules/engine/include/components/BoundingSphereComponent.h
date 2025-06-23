#pragma once

#include <glm/glm.hpp>

namespace RDE::Components {
    struct BoundingSphereComponent {
        float radius = 0.0f; // Radius of the bounding sphere
        glm::vec3 center = {0.0f, 0.0f, 0.0f}; // Center of the bounding sphere
    };
}
