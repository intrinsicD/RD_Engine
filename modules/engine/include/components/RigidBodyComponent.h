#pragma once

#include <glm/glm.hpp>

namespace RDE::Components {
    struct RigidBodyComponent {
        enum class BodyType { Static, Kinematic, Dynamic } type = BodyType::Dynamic; // Type of the rigid body

        glm::vec3 velocity = {0.0f, 0.0f, 0.0f}; // Linear velocity of the rigid body
        glm::vec3 angular_velocity = {0.0f, 0.0f, 0.0f}; // Angular velocity of the rigid body
        float mass = 1.0f; // Mass of the rigid body
        bool disable_gravity = false; // If true, the rigid body will not be affected by gravity
    };
}
