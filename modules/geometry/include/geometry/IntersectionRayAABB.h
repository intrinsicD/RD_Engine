#pragma once

#include "Ray.h"
#include "AABB.h"

namespace RDE{
    struct IntersectionRayAABB {
        bool hit;            // True if the ray intersects the AABB
        float tmin;         // Entry distance along the ray
        float tmax;         // Exit distance along the ray
    };

    IntersectionRayAABB Intersect(const Ray &ray, const AABB &aabb){
        IntersectionRayAABB result{false, 0.0f, 0.0f};
        float tmin = -std::numeric_limits<float>::infinity();
        float tmax =  std::numeric_limits<float>::infinity();
        for(int i=0;i<3;++i){
            float invD = 1.0f / (ray.direction[i] == 0.0f ? 1e-20f : ray.direction[i]);
            float t0 = (aabb.min[i] - ray.origin[i]) * invD;
            float t1 = (aabb.max[i] - ray.origin[i]) * invD;
            if(invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if(tmax < tmin) {
                result.hit = false;
                return result;
            }
        }
        result.hit = (tmax >= 0.0f); // intersection in front
        result.tmin = tmin;
        result.tmax = tmax;
        return result;
    }

}