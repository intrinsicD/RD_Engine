//ral/CommandBuffer.h
#pragma once

#include "Common.h"
#include "CommandBufferTypes.h"
#include "Resources.h"

#include <string>
#include <cstdint>

namespace RAL {
    // An abstract interface for a command buffer.
    // The implementation (e.g., VulkanCommandBuffer, OpenGLCommandBuffer) will inherit from this.
    class CommandBuffer {
    public:
        virtual ~CommandBuffer() = default;

        virtual void begin() = 0;

        virtual void end() = 0;

        virtual void begin_render_pass(const RenderPassDescription &desc) = 0;

        virtual void end_render_pass() = 0;

        virtual void set_viewport(const Viewport &viewport) = 0;

        virtual void set_scissor(const Rect2D &scissor) = 0;

        virtual void bind_pipeline(PipelineHandle pipeline) = 0;

        virtual void pipeline_barrier(const ResourceBarrier& barrier) = 0;

        virtual void bind_vertex_buffer(BufferHandle buffer, uint32_t binding) = 0;

        virtual void bind_index_buffer(BufferHandle buffer, IndexType indexType) = 0;

        virtual void bind_descriptor_set(PipelineHandle pipeline, DescriptorSetHandle set, uint32_t setIndex) = 0;

        virtual void copy_buffer(BufferHandle src, BufferHandle dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0) = 0;

        virtual void copy_buffer_to_texture(BufferHandle src, TextureHandle dst, const std::vector<BufferTextureCopy> &regions) = 0;

        virtual void push_constants(PipelineHandle pipeline, ShaderStage stages, uint32_t offset, uint32_t size, const void* data) = 0;


        virtual void draw(uint32_t vertex_count,
                          uint32_t instance_count = 1,
                          uint32_t first_vertex = 0,
                          uint32_t first_instance = 0) = 0;

        virtual void draw_indexed(uint32_t index_count,
                                  uint32_t instance_count = 1,
                                  uint32_t first_index = 0,
                                  int32_t vertex_offset = 0,
                                  uint32_t first_instance = 0) = 0;

        // Dispatches a compute shader.
        virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    };
} // namespace RAL