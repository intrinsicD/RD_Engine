#pragma once

#include "AABB.h"
#include "Sphere.h"
#include "Capsule.h"

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
