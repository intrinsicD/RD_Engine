#pragma once

#include <glm/glm.hpp>

namespace RDE{
    struct Ray{
        glm::vec3 origin; // The starting point of the ray
        glm::vec3 direction; // The direction vector of the ray, should be normalized

        Ray(const glm::vec3 &origin = glm::vec3(0.0f), const glm::vec3 &direction = glm::vec3(0.0f, 0.0f, -1.0f))
            : origin(origin), direction(glm::normalize(direction)) {}

        // Returns a point along the ray at a given distance t
        glm::vec3 at(float t) const {
            return origin + t * direction;
        }
    };

    inline glm::vec3 closest_point(const Ray &ray, const glm::vec3 &point) {
        const glm::vec3 to_point = point - ray.origin;
        const float t = glm::dot(to_point, ray.direction);
        return ray.at(t);
    }

    inline float squared_distance(const Ray &ray, const glm::vec3 &point) {
        const glm::vec3 closest = closest_point(ray, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float distance(const Ray &ray, const glm::vec3 &point) {
        return std::sqrt(squared_distance(ray, point));
    }
}