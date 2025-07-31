//assets/AssetComponentTypes.h
#pragma once

#include "core/Properties.h"
#include "AssetHandle.h"
#include "ral/Resources.h"
#include "components/DirtyTagComponent.h"

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

    struct AssetPipelineDescription {
        RAL::CullMode cullMode = RAL::CullMode::Back;
        RAL::PolygonMode polygonMode = RAL::PolygonMode::Fill;
        bool depthTest = true;
        bool depthWrite = true;
    };

    struct PrefabHierarchyComponent {
        AssetID parent;
        std::vector<AssetID> children; // Direct handles to child nodes' assets
    };

    struct RenderablePrototype {
        AssetID geometry_asset; // Reference to the geometry asset
        AssetID material_asset; // Reference to the material asset
    };

    struct AssetGeometrySubView {
        uint32_t index_offset{0}; // Offset in the geometry data
        uint32_t index_count{0}; // Number of indices in this sub-view
        int material_index{-1}; // Index of the material to use for this sub-view
        std::string name; // Optional name for the sub-view-material
        std::string material_name; // Optional name for the sub-view-material
    };

    struct AssetCpuGeometry {
        PropertyContainer vertices;
        PropertyContainer halfedges;
        PropertyContainer edges;
        PropertyContainer faces;
        PropertyContainer tets;

        std::vector<AssetGeometrySubView> subviews;

        size_t getVertexCount() const { return vertices.size(); }
    };

    template<>
    struct Dirty<AssetCpuGeometry> {
        std::vector<std::string> dirty_vertex_properties; // List of properties that are dirty
        std::vector<std::string> dirty_halfedge_properties; // List of properties that are dirty
        std::vector<std::string> dirty_edge_properties; // List of properties that are dirty
        std::vector<std::string> dirty_face_properties; // List of properties that are dirty
        std::vector<std::string> dirty_tets_properties; // List of properties that are dirty
    };

    struct AssetGpuGeometry {
        std::unordered_map<std::string, RAL::BufferHandle> buffers; //(e.g., position buffer, normal buffer, texcoord buffer, index buffer, ...)

        std::vector<AssetGeometrySubView> subviews;
    };

    struct AssetGpuTexture {
        RAL::TextureHandle texture; // GPU texture handle
        RAL::SamplerHandle sampler; // Sampler for texture filtering and wrapping

        int width{0}; // Texture width
        int height{0}; // Texture height
        int channels{0}; // Texture channels (e.g., RGB, RGBA)
        std::vector<char> data; // Pointer to the raw texture data, if needed
    };

    struct AssetShaderDef {
        std::string name;

        // Dependencies on the source code files
        std::unordered_map<std::string, std::vector<std::string>> dependencies;

        // List of features for shader permutations
        std::vector<std::string> features;

        // The GPU pipeline interface contract
        std::vector<RAL::VertexInputAttribute> vertexAttributes;
        std::vector<RAL::DescriptorSetLayoutDescription> descriptorSetLayouts;
        std::vector<RAL::PushConstantRange> pushConstantRanges;
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
