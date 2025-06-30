#pragma once

//#include "Properties.h"
#include "AssetHandle.h"
#include "ral/Common.h"

#include <string>
#include <vector>

namespace RDE {
    class PropertyContainer{};

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
        RAL::TextureHandle texture_id;
    };

    struct AssetGpuBuffer {
        RAL::BufferHandle buffer_id;
    };

    struct AssetGpuGeometry {
        RAL::BufferHandle vertex_buffer;
        RAL::BufferHandle index_buffer; // Optional

        uint32_t vertex_count = 0;
        uint32_t index_count = 0;   // Use this instead of element_count for clarity
        RAL::IndexType index_type = RAL::IndexType::UINT32; // Store the index type
    };

    struct AssetGpuPipeline {
        RAL::PipelineHandle pipeline_id; //the shader program handle
    };

    struct AssetTextSource {
        std::string source;
    };

    struct AssetPrefab {
        std::vector<AssetID> children;
    };
}
