//renderer/RendererComponentTypes.h
#pragma once

#include "ral/Common.h"
#include "ral/Resources.h"
#include "core/AttributeRegistry.h"

#include <unordered_map>

namespace RDE {
    struct RenderGpuGeometry {
        std::unordered_map<AttributeID, RAL::BufferHandle> attribute_buffers;

        /// @brief Handle to the optional index buffer.
        RAL::BufferHandle index_buffer = RAL::BufferHandle::INVALID();

        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
        RAL::IndexType index_type = RAL::IndexType::UINT32;
    };

    struct RenderGpuTexture {
        RAL::TextureHandle texture_id;
    };

    struct RenderGpuBuffer {
        RAL::BufferHandle buffer_id;
    };

    struct RenderGpuMaterial {
        RAL::PipelineHandle pipeline_id;
        RAL::DescriptorSetHandle descriptor_set_id; // Handle to the descriptor set for this material

        std::unordered_map<AttributeID, uint32_t> attribute_to_binding_map;
    };

    struct RenderGpuShaderStage {
        RAL::ShaderHandle handle;
        RAL::ShaderStage stage;
    };

    struct RenderGpuPipeline {
        RAL::PipelineHandle handle;
        std::vector<RenderGpuShaderStage> shader_stages; // List of shader stages in the pipeline
        RAL::DescriptorSetLayoutHandle descriptor_set_layout; // Layout used by this pipeline
    };
}
