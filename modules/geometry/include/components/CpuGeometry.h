//geometry/components/CpuGeometry.h
#pragma once

#include "core/Properties.h"

namespace RDE {
    struct CpuGeometryComponent {
        PropertyContainer vertices;
        PropertyContainer halfedges;
        PropertyContainer edges;
        PropertyContainer faces;
        PropertyContainer tets;
    };
}
