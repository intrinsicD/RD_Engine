#pragma once

#include "Properties.h"
#include "AssetHandle.h"

#include <string>

namespace RDE {
    using GpuHandle = unsigned int;
    // Placeholder for GPU resource handle. TODO: Need proper GpuHandle from the IGraphicsDevice here


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

    struct AssetGpuTexture {
        GpuHandle texture_id;
    };

    struct AssetGpuBuffer {
        GpuHandle buffer_id;
        size_t size; // Size in bytes
        int usage; // Usage hint (e.g., STATIC_DRAW, DYNAMIC_DRAW)
        int type; // Type of buffer (e.g., ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER)
    };

    struct AssetGpuGeometry {
        GpuHandle vao_id; // Vertex Array Object ID
        GpuHandle vbo_id; // Vertex Buffer Object ID
        GpuHandle ebo_id; // Element Buffer Object ID (optional)
        size_t element_count; // Number of indices (if using indexed rendering)
    };

    struct AssetShaderProgram {
        GpuHandle program_id;
    };

    struct AssetTextSource {
        std::string source;
    };

    struct AssetPrefab {
        std::vector<AssetHandle> children;
    };
}
