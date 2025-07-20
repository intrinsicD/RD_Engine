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

    struct AssetTextSource {
        std::string text; // The raw text content of the asset
    };

    struct AssetShaderModule {
        RAL::ShaderHandle module_handle; // Handle to the compiled shader
        RAL::ShaderStage stage; // Shader stage (vertex, fragment, etc.)
    };

    struct AssetPipeline {
        RAL::PipelineHandle pipeline_handle; // Handle to the shader program
        std::vector<AssetID> shaders; // List of shaders that make up this program
    };

    struct AssetMaterial {
        AssetID pipeline_asset; // Pipeline state for rendering

        PropertyContainer parameters; // Material parameters (e.g., color, metallic, roughness)

        std::unordered_map<std::string, AssetID> texture_bindings;
    };

    struct PrefabHierarchyComponent {
        AssetID parent;
        std::vector<AssetID> children; // Direct handles to child nodes' assets
    };

    struct RenderablePrototype {
        AssetID geometry_asset; // Reference to the geometry asset
        AssetID material_asset; // Reference to the material asset
    };

    struct AssetCpuGeometry {
        PropertyContainer vertices;
        PropertyContainer halfedges;
        PropertyContainer edges;
        PropertyContainer faces;
        PropertyContainer tets;
    };

    struct AssetGpuGeometry {
        std::unordered_map<std::string, RAL::BufferHandle> buffers; //(e.g., position buffer, normal buffer, texcoord buffer, index buffer, ...)
    };

    struct AssetGpuTexture {
        RAL::TextureHandle texture; // GPU texture handle
        RAL::SamplerHandle sampler; // Sampler for texture filtering and wrapping

        int width{0}; // Texture width
        int height{0}; // Texture height
        int channels{0}; // Texture channels (e.g., RGB, RGBA)
        std::vector<char> data; // Pointer to the raw texture data, if needed
    };

    struct AssetCpuShaderDefinition {
        std::string name; // Name of the shader definition
        std::unordered_map<RAL::ShaderStage, std::string> base_spirv_paths; // Base paths for SPIR-V files for each stage

        // Dependencies are now just URIs, no need for separate source/spirv lists.
        std::vector<std::string> dependencies; // URIs of dependencies (e.g., other shaders, textures)

        RAL::CullMode cull_mode;
        bool depth_test{true}; // Enable depth testing
        bool depth_write{true}; // Enable depth writing
        //TODO add more pipeline state options as needed

        //vertex layout
        std::vector<RAL::VertexInputAttribute> vertex_layout; // Vertex attributes (location, format, offset)
        PropertyContainer state;
    };

    struct Prefab {
        struct Node {
            AssetID mesh_asset;   // Direct handle, not an index
            AssetID material_asset; // Direct handle
            std::string name;
            glm::mat4 transform; // Store the transform directly
            AssetID parent;      // Direct handle to parent node's asset
            std::vector<AssetID> children;
        };

        // No more separate vectors for meshes/materials/transforms.
        // Each node is self-contained.
        std::vector<Node> nodes;
        std::vector<AssetID> root_nodes;
    };
}
