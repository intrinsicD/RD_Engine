//assets/AssetComponentTypes.h
#pragma once

#include "core/Properties.h"
#include "AssetHandle.h"

#include <string>
#include <vector>

namespace RDE {
    struct AssetFilepath {
        std::string path;
    };

    struct AssetName {
        std::string name;
    };

    struct AssetCpuGeometry {
        PropertyContainer vertices;
        PropertyContainer halfedges; // Optional, used for halfedge-meshes and graphs
        PropertyContainer edges; // Optional, used for halfedge-meshes and graphs
        PropertyContainer faces; // Optional, used for meshes
    };

    struct AssetCpuTexture {
        std::vector<u_int8_t> data;
        int width;
        int height;
        int channels; // Number of color channels (e.g., 3 for RGB, 4 for RGBA)
    };

    struct AssetTextSource {
        std::string source;
    };

    struct AssetPrefab {
        std::vector<AssetID> children;
    };
}
