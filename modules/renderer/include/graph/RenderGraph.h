#pragma once

#include "RenderPassBuilder.h"
#include "RenderGraphResource.h"
#include "ral/CommandBuffer.h"

#include <vector>
#include <string>
#include <memory>


namespace RDE {
    class RenderGraph {
    public:
        RenderGraph() = default;

        // The main entry point for adding a render pass.
        // The user provides a name and a setup lambda.
        void add_pass(const std::string& name, std::function<void(RenderPassBuilder&)>&& setup_func);

        // Imports an existing, external resource (like the backbuffer) into the graph.
        RGTextureHandle import_texture(const std::string& name, RAL::TextureHandle external_texture);

        // Analyzes dependencies, culls passes, allocates physical resources,
        // and calculates barriers.
        void compile();

        // Executes the compiled graph using the provided command buffer.
        void execute(RAL::CommandBuffer& command_buffer);

        // Clears all passes and resources, ready for the next frame.
        void clear();

    private:
        friend class RenderPassBuilder;

        // Internal data structures would be defined here. For example:
        struct RenderPassNode {
            std::string name;
            std::function<void(RenderPassBuilder&)> setup;
            std::function<void(RAL::CommandBuffer&)> execute;
            std::vector<RGBufferHandle> buffer_reads;
            std::vector<RGTextureHandle> texture_reads;
            std::vector<RGTextureHandle> texture_writes;
            // ... and more metadata for the compiler
        };

        std::vector<RenderPassNode> m_passes;
        std::vector<RGTextureDescription> m_texture_resources; // And buffer resources
        // ... many more internal details for the compiled state ...
    };
}
