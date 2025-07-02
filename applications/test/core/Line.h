#pragma once

#include <glm/glm.hpp>
#include <optional>

namespace RDE{
    struct Line {
        glm::vec3 base;      // A point on the line (specifically, the point closest to the origin).
        glm::vec3 direction; // The normalized direction vector of the line.

        glm::vec3 at(float t) const {
            return base + t * direction; // Returns a point on the line at distance t from the base point.
        }
    };

    inline glm::vec3 closest_point(const Line &line, const glm::vec3 &point) {
        // Project the point onto the line
        const float t = glm::dot(point - line.base, line.direction);
        return line.base + t * line.direction;
    }

    inline float squared_distance(const Line &line, const glm::vec3 &point) {
        const glm::vec3 closest = closest_point(line, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float distance(const Line &line, const glm::vec3 &point) {
        return std::sqrt(squared_distance(line, point));
    }

    inline bool contains(const Line &line, const glm::vec3 &point) {
        // A point is considered "contained" if it lies on the line
        return squared_distance(line, point) < 1e-6f; // Use a small epsilon for floating point comparison
    }

    inline bool intersects(const Line &line, const glm::vec3 &point) {
        // A point intersects the line if it is not on the line
        return !contains(line, point);
    }

    inline std::optional<glm::vec3> intersection(const Line& line1, const Line& line2) {
        const float epsilon = 1e-6f;

        const glm::vec3 delta_base = line2.base - line1.base;
        const glm::vec3 cross_d1_d2 = glm::cross(line1.direction, line2.direction);
        const float cross_mag_sq = glm::dot(cross_d1_d2, cross_d1_d2);

        // 1. Check if lines are parallel.
        if (cross_mag_sq < epsilon) {
            // Lines are parallel. They intersect only if they are collinear.
            // Check if the base points are on the same line.
            const float delta_base_cross_d1_mag_sq = glm::dot(glm::cross(delta_base, line1.direction),
                                                        glm::cross(delta_base, line1.direction));
            if (delta_base_cross_d1_mag_sq < epsilon) {
                // Lines are collinear. There are infinite intersection points.
                // A single point cannot be returned.
                return std::nullopt;
            }
            // Lines are parallel and non-intersecting.
            return std::nullopt;
        }

        // 2. Check if lines are skew.
        //    If the scalar triple product is non-zero, the lines are not coplanar.
        const float triple_product = glm::dot(delta_base, cross_d1_d2);
        if (std::abs(triple_product) > epsilon) {
            // Lines are skew and do not intersect.
            return std::nullopt;
        }

        // 3. Lines are coplanar and not parallel, so they must intersect.
        //    Calculate the parameter 't' for the intersection point on line1.
        //    P = line1.base + t * line1.direction
        //    This is a known and robust formula for the intersecting case.
        const float t = glm::dot(glm::cross(delta_base, line2.direction), cross_d1_d2) / cross_mag_sq;

        return line1.at(t);
    }
}