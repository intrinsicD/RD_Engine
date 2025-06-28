#pragma once

#include "RenderGraphResource.h"
#include "RendererTypes.h"

namespace RDE {
    class RenderGraph;
    class RGPass;

    // The interface a render pass uses to declare its inputs and outputs.
    // This is a "façade" that limits what a pass can do during setup.
    class RGBuilder {
    public:
        RGBuilder(RenderGraph& graph, RGPass& pass);

        // Declares that this pass reads from a resource.
        void read(RGResourceHandle handle);

        // Declares that this pass writes to a resource.
        RGResourceHandle write(RGResourceHandle handle);

        // Creates a new virtual resource (e.g., a render target)
        RGResourceHandle create_texture(const struct TextureDesc& desc);

    private:
        RenderGraph& m_graph;
        RGPass& m_pass;
    };
}