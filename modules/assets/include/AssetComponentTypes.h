#pragma once

//#include "Properties.h"
#include "AssetHandle.h"
#include "AttributeRegistry.h"
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
        /// @brief get the attribute id from the attribute registry
        std::unordered_map<AttributeID, RAL::BufferHandle> attribute_buffers;

        /// @brief Handle to the optional index buffer.
        RAL::BufferHandle index_buffer;

        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
        RAL::IndexType index_type = RAL::IndexType::UINT32;
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
