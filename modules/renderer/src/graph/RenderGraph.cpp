#include "graph/RenderGraph.h"
#include "graph/RenderPassBuilder.h"
#include "graph/RenderGraphResource.h"
#include "ral/CommandBuffer.h"
#include "ral/Device.h"
#include "ral/RenderPass.h"
#include "ral/Barrier.h"
#include "ral/DescriptorSet.h"

namespace RDE{
    void RenderGraph::add_pass(const std::string& name, std::function<void(RenderPassBuilder&)>&& setup_func) {
        RenderPassNode node;
        node.name = name;
        node.setup = std::move(setup_func);
        m_passes.push_back(node);
    }

    RGTextureHandle RenderGraph::import_texture(const std::string& name, RAL::TextureHandle external_texture) {
        return RGTextureHandle{static_cast<uint32_t>(m_texture_resources.size() - 1)};
    }

    void RenderGraph::compile() {
        // Analyze dependencies, cull passes, allocate resources, etc.
    }

    void RenderGraph::execute(RAL::CommandBuffer& command_buffer) {
        // Execute the compiled graph using the provided command buffer.
    }

    void RenderGraph::clear() {
        m_passes.clear();
        m_texture_resources.clear();
    }
}