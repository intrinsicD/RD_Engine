#pragma once

#include "Segment.h"

namespace RDE {
    struct Capsule {
        Segment segment; // The segment that defines the capsule
        float radius;    // Radius of the capsule

        bool is_valid() const {
            return radius >= 0.0f;
        }

        float volume() const {
            // Volume of a capsule = volume of cylinder + volume of two hemispheres
            const float cylinder_volume = 3.14159265358979323846f * radius * radius * segment.length();
            const float sphere_volume = (4.0f / 3.0f) * 3.14159265358979323846f * radius * radius * radius;
            return cylinder_volume + sphere_volume;
        }

        float surface_area() const {
            // Surface area of a capsule = surface area of cylinder + surface area of two hemispheres
            const float cylinder_area = 2.0f * 3.14159265358979323846f * radius * segment.length();
            const float sphere_area = 4.0f * 3.14159265358979323846f * radius * radius;
            return cylinder_area + sphere_area;
        }
    };

    inline Capsule Merge(const Capsule &a, const Capsule &b) {
        // Merge two capsules by creating a new capsule that encompasses both
        const glm::vec3 new_start = glm::min(a.segment.start, b.segment.start);
        const glm::vec3 new_end = glm::max(a.segment.end, b.segment.end);
        const float new_radius = glm::max(a.radius, b.radius);
        return {Segment{new_start, new_end}, new_radius};
    }

    inline glm::vec3 ClosestPoint(const Capsule &capsule, const glm::vec3 &point) {
        // 1. Find the closest point on the capsule's core segment.
        const glm::vec3 closest_on_segment = ClosestPoint(capsule.segment, point);

        // 2. Calculate the vector from the segment to the point.
        const glm::vec3 diff = point - closest_on_segment;
        const float dist_sq = glm::dot(diff, diff);

        // 3. Handle the edge case where the point is on the core segment.
        if (dist_sq < 1e-8f) { // Use a small epsilon for floating point safety
            // The point is on the axis. We need to push it outwards.
            // We need an arbitrary direction that is not parallel to the segment.
            const glm::vec3 segment_dir = glm::normalize(capsule.segment.end - capsule.segment.start);
            glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);

            // If the segment is aligned with the 'up' vector, use a different one.
            if (glm::abs(glm::dot(segment_dir, up_vector)) > 0.999f) {
                up_vector = glm::vec3(1.0f, 0.0f, 0.0f);
            }

            // Use the cross product to get a perpendicular direction.
            const glm::vec3 outward_dir = glm::normalize(glm::cross(segment_dir, up_vector));
            return closest_on_segment + outward_dir * capsule.radius;
        }

        // 4. For all other points, normalize the diff vector and scale by the radius.
        return closest_on_segment + (diff / std::sqrt(dist_sq)) * capsule.radius;
    }

    inline float SquaredDistance(const Capsule &capsule, const glm::vec3 &point) {
        const glm::vec3 closest = ClosestPoint(capsule, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float Distance(const Capsule &capsule, const glm::vec3 &point) {
        return std::sqrt(SquaredDistance(capsule, point));
    }

    inline bool Contains(const Capsule &capsule, const glm::vec3 &point) {
        return SquaredDistance(capsule, point) <= capsule.radius * capsule.radius;
    }

    inline bool Intersects(const Capsule &a, const Capsule &b) {
        // Check if the distance between the closest points on the segments is less than or equal to the sum of the radii
        return Distance(a, b.segment.start) <= (a.radius + b.radius) ||
               Distance(a, b.segment.end) <= (a.radius + b.radius) ||
               Distance(b, a.segment.start) <= (b.radius + a.radius) ||
               Distance(b, a.segment.end) <= (b.radius + a.radius);
    }
}