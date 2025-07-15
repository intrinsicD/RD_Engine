//assets/AssetComponentTypes.h
#pragma once

#include "core/Properties.h"
#include "AssetHandle.h"
#include "ral/Resources.h"

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

    struct AssetCpuMaterial {
        std::string name;
        std::string shader_path; // Path to the shader file
        RAL::CullMode cull_mode = RAL::CullMode::Back; // Default cull mode
        bool depth_test = true; // Default depth test enabled
        bool depth_write = true; // Default depth write enabled
        std::unordered_map<std::string, glm::vec4> vector_params; // Vector parameters (e.g., base color)
        std::unordered_map<std::string, float> float_params; // Float parameters (e.g., metalness, roughness)
        std::unordered_map<std::string, AssetID> texture_asset_ids; // Maps sampler names to texture asset IDs
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

    struct AssetMetadata {
        AssetID default_material;
    };

    struct AssetPrefab {
        std::vector<AssetID> child_assets; // Asset IDs of child assets
    };
}
