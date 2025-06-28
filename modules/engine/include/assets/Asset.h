#pragma once

#include <string>
#include <vector>

namespace RDE {
    struct AssetFilepath {
        std::string path;
    };

    struct AssetCpuGeometry {
        std::vector<> vertices; // Replace with actual vertex type
        std::vector<> indices;  // Replace with actual index type
    };
}