#pragma once

#include <glm/glm.hpp>

namespace RDE{
    struct Segment {
        glm::vec3 start; // Start point of the segment
        glm::vec3 end;   // End point of the segment

        bool is_valid() const {
            return start != end; // A valid segment must have distinct start and end points
        }

        float length() const {
            return glm::length(end - start); // Length of the segment is the distance between start and end points
        }
    };

    inline glm::vec3 ClosestPoint(const Segment& segment, const glm::vec3& point) {
        glm::vec3 segment_dir = segment.end - segment.start;
        float length_sq = glm::dot(segment_dir, segment_dir);
        if (length_sq < 1e-6f) return segment.start; // Segment is a point

        // Project point onto the line defined by the segment.
        // The parameter t is clamped to [0, 1] to stay on the segment.
        float t = glm::clamp(glm::dot(point - segment.start, segment_dir) / length_sq, 0.0f, 1.0f);
        return segment.start + t * segment_dir;
    }
}