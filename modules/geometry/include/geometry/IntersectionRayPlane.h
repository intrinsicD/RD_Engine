#pragma once

#include "Ray.h"
#include "Plane.h"
#include "core/Constants.h"

namespace RDE{
    struct IntersectionRayPlane {
        bool hit = false; // Indicates if the ray intersects the plane
        float distance = INFINITY; // Distance from the ray origin to the intersection point
    };

    IntersectionRayPlane Intersect(const Ray &ray, const Plane &plane, float eps = 1e-6f, float tmin = 0.0f, float tmax = INFINITY) {
        const float denom = dot(plane.normal, ray.direction);          // n·v
        const float num   = dot(plane.normal, ray.origin) + plane.distance;    // n·o + d

        // Parallel or nearly-parallel: no single intersection
        if (fabsf(denom) < eps) {
            // Optional: treat coplanar hits here if fabsf(num) < eps
            return {};
        }

        const float t = -num / denom;
        if (t < tmin || t > tmax) return {};
        return {true, t};
    }
}