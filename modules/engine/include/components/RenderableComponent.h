#pragma once

#include "assets/AssetHandle.h"

namespace RDE::Components {
    struct RenderableComponent {
        AssetHandle material_handle;
        AssetHandle geometry_handle;

        enum Type {
            Mesh, Graph, PointCloud
        }type;
    };
}
