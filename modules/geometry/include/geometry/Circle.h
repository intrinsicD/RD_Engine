#pragma once
#include <glm/glm.hpp>

namespace RDE{
    struct Circle{
        glm::vec3 center; // Center of the circle
        glm::vec3 normal; // Normal vector of the plane the circle lies on
        float radius;     // Radius of the circle

        float perimeter() const {
            return 2.0f * 3.14159265358979323846f * radius; // Perimeter of the circle
        }

        float area() const {
            return 3.14159265358979323846f * radius * radius; // Area of the circle
        }
    };
}