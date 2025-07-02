#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <optional>

namespace RDE {
    struct AABB {
        static AABB Create(const glm::vec3 &point) {
            return {point, point};
        }

        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

        bool is_valid() const {
            return min.x <= max.x && min.y <= max.y && min.z <= max.z;
        }

        glm::vec3 diagonal() const {
            return max - min;
        }

        glm::vec3 half_extent() const {
            return diagonal() * 0.5f;
        }

        glm::vec3 center() const {
            return (min + max) * 0.5f;
        }

        float volume() const {
            const glm::vec3 diag = diagonal();
            return diag.x * diag.y * diag.z;
        }

        void clear() {
            min = glm::vec3(std::numeric_limits<float>::max());
            max = glm::vec3(std::numeric_limits<float>::lowest());
        }
    };

    inline AABB Merge(const AABB &a, const AABB &b) {
        AABB result;
        result.min = glm::min(a.min, b.min);
        result.max = glm::max(a.max, b.max);
        return result;
    }

    inline glm::vec3 ClosestPoint(const AABB &aabb, const glm::vec3 &point) {
        return glm::clamp(point, aabb.min, aabb.max);
    }

    inline float SquaredDistance(const AABB &aabb, const glm::vec3 &point) {
        const glm::vec3 closest = ClosestPoint(aabb, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float Distance(const AABB &aabb, const glm::vec3 &point) {
        return std::sqrt(SquaredDistance(aabb, point));
    }

    inline bool Contains(const AABB &aabb, const glm::vec3 &point) {
        return point.x >= aabb.min.x && point.x <= aabb.max.x &&
               point.y >= aabb.min.y && point.y <= aabb.max.y &&
               point.z >= aabb.min.z && point.z <= aabb.max.z;
    }

    inline bool Intersects(const AABB &a, const AABB &b) {
        return !(a.max.x < b.min.x || b.max.x < a.min.x ||
                 a.max.y < b.min.y || b.max.y < a.min.y ||
                 a.max.z < b.min.z || b.max.z < a.min.z);
    }

    inline std::optional<AABB> Intersection(const AABB &a, const AABB &b) {
        if (!Intersects(a, b)) {
            return std::nullopt; // Return an empty AABB if no intersection
        }
        return AABB{glm::max(a.min, b.min), glm::min(a.max, b.max)};
    }
}