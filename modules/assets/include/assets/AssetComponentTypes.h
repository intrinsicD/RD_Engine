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

    struct AssetCpuShaderProgram {
        //Shading
        AssetID vertex_shader;
        AssetID fragment_shader;
        AssetID geometry_shader; // Optional, for geometry shaders
        AssetID tessellation_control_shader; // Optional, for tessellation control shaders
        AssetID tessellation_evaluation_shader; // Optional, for tessellation evaluation shaders

        //Compute
        AssetID compute_shader;

        // Ray Tracing
        /*        AssetID raygen_shader; // Optional, for ray tracing
        AssetID raymiss_shader; // Optional, for ray tracing
        AssetID rayhit_closest_shader; // Optional, for ray tracing
        AssetID rayhit_any_shader; // Optional, for ray tracing
        AssetID intersection_shader; // Optional, for ray tracing*/

        // Task and Mesh Shaders
        AssetID task_shader; // Optional, for task shaders
        AssetID mesh_shader; // Optional, for mesh shaders
    };

    struct AssetSpirvBytecode {
        std::vector<uint32_t> bytecode; // SPIR-V bytecode
        RAL::ShaderStage stage; // Shader stage (e.g., Vertex, Fragment)
    };

    struct VertexAttributeDesc {
        std::string semantic_name; // e.g., "POSITION", "NORMAL"
        RAL::Format format;
    };

    struct AssetCpuShaderDefinition {
        std::string name;
        std::unordered_map<RAL::ShaderStage, std::string> base_spirv_paths;
        std::vector<std::string> features; // Permutation defines like "HAS_NORMAL_MAP"

        // Fixed pipeline state for all permutations
        RAL::CullMode cull_mode = RAL::CullMode::Back;
        bool depth_test = true;
        bool depth_write = true;
        // ... add blend modes, topology, etc. here ...

        // The vertex layout this shader program expects
        std::vector<VertexAttributeDesc> vertex_layout;
    };

    struct AssetMetadata {
        AssetID default_material;
    };

    struct AssetPrefab {
        std::vector<AssetID> child_assets; // Asset IDs of child assets
    };
}
