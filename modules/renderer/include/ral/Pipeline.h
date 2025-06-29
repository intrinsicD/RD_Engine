#pragma once
#include "Common.h"
#include "DescriptorSet.h"
#include <vector>

namespace RAL {
    struct ShaderModule {
        std::vector<char> spirv_code;
        ShaderStageFlags stage;
    };

    struct VertexInputBindingDescription { /* describes vertex buffer stride */ };
    struct VertexInputAttributeDescription { /* describes location, format, offset of an attribute */ };

    struct PipelineVertexInputState {
        std::vector<VertexInputBindingDescription> bindings;
        std::vector<VertexInputAttributeDescription> attributes;
    };
    
    struct PipelineRasterizationState { /* cull mode, polygon mode, line width, etc. */ };
    struct PipelineDepthStencilState { /* depth test/write enable, compare op, etc. */ };
    struct PipelineColorBlendAttachmentState { /* blend enable, src/dst factors, etc. */ };
    struct PipelineColorBlendState {
        std::vector<PipelineColorBlendAttachmentState> attachments;
    };

    // The complete description for a graphics pipeline.
    struct PipelineDescription {
        std::vector<ShaderModule> shaders;
        std::vector<DescriptorSetLayoutHandle> descriptor_set_layouts;
        
        PipelineVertexInputState vertex_input_state;
        // PipelineInputAssemblyState (primitive topology)
        PipelineRasterizationState rasterization_state;
        // PipelineMultisampleState
        PipelineDepthStencilState depth_stencil_state;
        PipelineColorBlendState color_blend_state;
    };
}