#include "graph/RenderPassBuilder.h"
#include "graph/RenderGraph.h"
#include "graph/RenderGraphPass.h"
#include "graph/RenderGraphResource.h"

namespace RDE {
    RGBuilder::RGBuilder(RenderGraph &graph, RGPass &pass)
            : m_graph(graph), m_pass(pass) {}

    void RGBuilder::read(RGResourceHandle resource) {
        m_pass.m_reads.push_back(resource);
        // When a resource is read, its reference count is incremented.
        // This is key for the culling algorithm.
        m_graph.m_resources[resource.index].ref_count++;
    }

    RGResourceHandle RGBuilder::write(RGResourceHandle resource) {
        RGPass *producer = m_graph.m_passes[m_graph.m_resources[resource.index].producer_pass_idx].get();
        if (producer == nullptr) {
            // This should handle "imported" resources, but for now we'll throw.
            throw std::runtime_error("Cannot write to a resource with no producer.");
        }

        m_pass.m_writes.push_back(resource);
        // Mark this pass as the new "producer" of this resource version.
        // Note: For a more advanced graph, this would create a *new* version of the
        // resource, but for simplicity, we'll just overwrite the producer.
        m_graph.m_resources[resource.index].producer_pass_idx = m_pass.m_index;
        return resource;
    }

    RGResourceHandle RGBuilder::create_texture(const TextureDesc &desc) {
        // Delegate the actual creation to the graph.
        RGResourceHandle handle = m_graph.create_virtual_texture(desc);

        // The creating pass is automatically the producer and gets write access.
        m_pass.m_writes.push_back(handle);
        m_graph.m_resources[handle.index].producer_pass_idx = m_pass.m_index;
        return handle;
    }
}