#pragma once

#include "RenderGraphResource.h"
#include "ral/CommandBuffer.h"

#include <functional>

namespace RDE {
    class RenderGraph;

    class RenderPassBuilder {
    public:
        // Declare that this pass reads from a resource.
        void read(RGBufferHandle handle);
        void read(RGTextureHandle handle);

        // Declare that this pass writes to a resource.
        // This is typically used for render targets.
        void write(RGTextureHandle handle);

        // A special version of write for the first time a resource is used.
        // This effectively "creates" the virtual resource.
        RGBufferHandle create_buffer(const RGBufferDescription& desc);
        RGTextureHandle create_texture(const RGTextureDescription& desc);

        // Set the lambda containing the actual GPU commands to run for this pass.
        void set_execute_callback(std::function<void(RAL::CommandBuffer&)>&& execute_func);

    private:
        friend class RenderGraph;
        RenderPassBuilder(RenderGraph& graph, uint32_t pass_index);

        RenderGraph& m_graph;
        uint32_t m_pass_index;
    };
}