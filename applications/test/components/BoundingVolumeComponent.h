#pragma once

#include "geometry/AABB.h"
#include "geometry/Sphere.h"
#include "geometry/Capsule.h"

namespace RDE {
    struct BoundingVolumeDirty{};

    struct BoundingVolumeAABBComponent {
        AABB world;
        AABB local;
    };

    struct BoundingVolumeSphereComponent {
        Sphere world;
        Sphere local;
    };

    struct BoundingVolumeCapsuleComponent {
        Capsule world;
        Capsule local;
    };
}
