#pragma once

#include "Circle.h"

#include <optional>

namespace RDE {
    struct Sphere {
        static Sphere Create(const glm::vec3 &center) {
            return Sphere(center, 0.0f);
        }

        glm::vec3 center = glm::vec3(0.0f); // Center of the sphere
        float radius = 0.0f; // Radius of the sphere

        bool is_valid() const {
            return radius >= 0.0f;
        }

        float volume() const {
            return (4.0f / 3.0f) * 3.14159265358979323846f * radius * radius * radius;
        }

        float surface_area() const {
            return 4.0f * 3.14159265358979323846f * radius * radius;
        }
    };

    inline Sphere Merge(const Sphere &a, const Sphere &b) {
        const glm::vec3 new_center = (a.center + b.center) * 0.5f;
        const float new_radius = glm::length(a.center - b.center) * 0.5f + glm::max(a.radius, b.radius);
        return {new_center, new_radius};
    }

    inline glm::vec3 ClosestPoint(const Sphere &sphere, const glm::vec3 &point) {
        const glm::vec3 direction = glm::normalize(point - sphere.center);
        return sphere.center + direction * sphere.radius;
    }

    inline float SquaredDistance(const Sphere &sphere, const glm::vec3 &point) {
        const glm::vec3 closest = ClosestPoint(sphere, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float Distance(const Sphere &sphere, const glm::vec3 &point) {
        return std::sqrt(SquaredDistance(sphere, point));
    }

    inline bool Contains(const Sphere &sphere, const glm::vec3 &point) {
        return SquaredDistance(sphere, point) <= sphere.radius * sphere.radius;
    }

    inline bool Intersects(const Sphere &a, const Sphere &b) {
        return glm::length(a.center - b.center) <= (a.radius + b.radius);
    }

    inline std::optional<Circle> Intersection(const Sphere &a, const Sphere &b) {
        glm::vec3 vec_between_centers = b.center - a.center;
        float dist_sq = glm::dot(vec_between_centers, vec_between_centers);
        float dist = glm::sqrt(dist_sq);

        // --- Pre-condition Checks for Intersection ---
        // 1. Check if spheres are too far apart to intersect.
        if (dist > a.radius + b.radius) {
            return std::nullopt; // No intersection
        }

        // 2. Check if one sphere is contained within the other without intersecting.
        if (dist < std::abs(a.radius - b.radius)) {
            return std::nullopt; // No intersection, one is inside the other
        }

        // --- Calculate Intersection Properties ---
        Circle result;

        // The normal of the intersection circle is the vector between the centers.
        result.normal = glm::normalize(vec_between_centers);

        // Use the law of cosines to find the distance from sphere a's center to the
        // center of the intersection circle. Let this be 'd_a'.
        // a.radius^2 = d_a^2 + circle_radius^2
        // b.radius^2 = (dist - d_a)^2 + circle_radius^2
        // Solving for d_a gives:
        float d_a = (dist_sq - b.radius * b.radius + a.radius * a.radius) / (2.0f * dist);

        // The center of the intersection circle is found by moving from sphere a's center
        // along the vector towards sphere b's center by distance d_a.
        result.center = a.center + result.normal * d_a;

        // Now calculate the radius of the intersection circle using the Pythagorean theorem.
        result.radius = glm::sqrt(a.radius * a.radius - d_a * d_a);

        return result;
    }
}