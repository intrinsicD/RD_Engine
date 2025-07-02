#pragma once

#include "AABB.h"
#include "Sphere.h"
#include "Capsule.h"

namespace RDE::BoundingVolume {
    struct Dirty{};

    struct AABBComponent {
        AABB world;
        AABB local;
    };

    struct SphereComponent {
        Sphere world;
        Sphere local;
    };

    struct CapsuleComponent {
        Capsule world;
        Capsule local;
    };
}
