#pragma once
#include "glm/vec3.hpp"

namespace RDE::Components {
    struct ColliderComponent {
        enum class ShapeType { Box, Sphere, Capsule, ConvexMesh, TriangleMesh } type = ShapeType::Box;
        glm::vec3 offset = glm::vec3(0.0f);
        AssetHandle physics_material;
    };
}
