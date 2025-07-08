#pragma once

#include "Line.h"

#include <optional>

namespace RDE {
    struct Plane {
        glm::vec3 normal;
        float distance; // Distance from origin along the normal
    };

    inline glm::vec3 closest_point(const Plane &plane, const glm::vec3 &point) {
        // Project the point onto the plane
        const float d = glm::dot(plane.normal, point) - plane.distance;
        return point - d * plane.normal;
    }

    inline float squared_distance(const Plane &plane, const glm::vec3 &point) {
        const glm::vec3 closest = closest_point(plane, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float distance(const Plane &plane, const glm::vec3 &point) {
        return std::sqrt(squared_distance(plane, point));
    }

    inline bool contains(const Plane &plane, const glm::vec3 &point) {
        // A point is considered "contained" if it lies on the plane
        return std::abs(glm::dot(plane.normal, point) - plane.distance) < 1e-6f;
    }

    inline bool intersects(const Plane &plane, const glm::vec3 &point) {
        // A point intersects the plane if it is not on the plane
        return !contains(plane, point);
    }

    inline std::optional<Line> intersect(const Plane &a, const Plane &b) {
        // 1. Calculate the direction of the intersection line.
        glm::vec3 direction = glm::cross(a.normal, b.normal);

        // 2. Check if the planes are parallel.
        //    We check the squared length to avoid a sqrt().
        float det = glm::dot(direction, direction);
        if (det < 1e-8f) { // Use a small epsilon for floating point comparison
            return std::nullopt;
        }

        // 3. Find a point on the line.
        //    This formula calculates the point on the intersection line closest to the origin.
        //    It is derived by solving the system of equations for the two planes plus a
        //    third plane that passes through the origin and is orthogonal to the line.
        glm::vec3 base = (glm::cross(direction, b.normal) * a.distance +
                          glm::cross(a.normal, direction) * b.distance) / det;

        // The result is a point on the line and the line's direction.
        // Normalizing the direction is good practice.
        return Line{base, glm::normalize(direction)};
    }
}