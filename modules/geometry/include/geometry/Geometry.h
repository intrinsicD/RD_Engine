#pragma once

#include "core/Properties.h"

namespace RDE{
    struct Geometry{
        PropertyContainer vertices;     // Vertices of the geometry
        PropertyContainer halfedges;    // Optional, used for half-edge structures
        PropertyContainer edges;        // Optional, used for edge structures
        PropertyContainer faces;        // Optional, used for polygonal meshes
        PropertyContainer tets;         // Optional, used for tetrahedral meshes
    };
}