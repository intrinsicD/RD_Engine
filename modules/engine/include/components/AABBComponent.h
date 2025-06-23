#pragma once

#include <glm/glm.hpp>

namespace RDE::Components {
    struct AABBLocalComponent {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct AABBWorldComponent {
        glm::vec3 min;
        glm::vec3 max;
    };
}
