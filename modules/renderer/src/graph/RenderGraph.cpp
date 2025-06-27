#include "graph/RenderGraph.h"
#include "graph/RenderGraphBuilder.h"
#include "Log.h"

#include <stack>

namespace RDE{
    void RenderGraph::add_pass(std::string_view name, RGPass::SetupFunc setup, RGPass::ExecuteFunc execute) {
        // Create the pass and add it to the graph's list.
        auto pass = std::make_unique<RGPass>(std::string(name), setup, execute);
        pass->m_index = static_cast<uint32_t>(m_passes.size());
        m_passes.push_back(std::move(pass));

        // Immediately call the setup function with a builder.
        // This is where the pass declares all its inputs and outputs.
        RGBuilder builder(*this, *m_passes.back());
        setup(builder);
    }

    void RenderGraph::execute(ICommandBuffer& command_buffer, const RenderPacket& packet, IGraphicsDevice* device) {
        // The three main phases of the Render Graph
        cull_passes();
        allocate_physical_resources(device);
        record_pass_commands(command_buffer, packet);

        // After execution, we could deallocate the transient physical resources.
        // (Implementation omitted for brevity, but would call device->destroy_texture).
    }

    RGResourceHandle RenderGraph::create_virtual_texture(const TextureDesc& desc) {
        RGResourceHandle handle;
        handle.index = static_cast<uint32_t>(m_resources.size());

        RGVirtualResource resource;
        resource.desc = desc;
        resource.ref_count = 0; // Starts with zero refs.
        m_resources.push_back(resource);

        return handle;
    }

    void RenderGraph::cull_passes() {
        // This is a simplified culling algorithm.
        // A more robust version would use a proper graph traversal (e.g., Depth First Search).

        std::stack<uint32_t> pass_stack;

        // Assumption: We need to define which resources are the "graph outputs".
        // For now, let's assume any resource that is ever written to but never read
        // by another pass is an output we want to keep.
        // A better way is to have an explicit `graph.set_output(resource)` call.
        for (size_t i = 0; i < m_resources.size(); ++i) {
            if (m_resources[i].ref_count == 0 && m_resources[i].producer_pass_idx != -1) {
                // This resource is written to but never read. Assume it's a final output.
                pass_stack.push(m_resources[i].producer_pass_idx);
                m_passes[m_resources[i].producer_pass_idx]->m_is_culled = false;
            }
        }

        // Traverse backwards from the output passes to find all dependencies.
        while(!pass_stack.empty()) {
            uint32_t pass_idx = pass_stack.top();
            pass_stack.pop();

            RGPass* pass = m_passes[pass_idx].get();

            for (const auto& read_handle : pass->m_reads) {
                int32_t producer_idx = m_resources[read_handle.index].producer_pass_idx;
                if (producer_idx != -1 && m_passes[producer_idx]->m_is_culled) {
                    m_passes[producer_idx]->m_is_culled = false;
                    pass_stack.push(producer_idx);
                }
            }
        }

        // Now, build the final, ordered list of passes to execute.
        // (A topological sort is the correct way, but for now we'll assume pass order is ok).
        for (const auto& pass : m_passes) {
            if (!pass->m_is_culled) {
                m_culled_passes.push_back(pass.get());
            }
        }
    }

    void RenderGraph::allocate_physical_resources(IGraphicsDevice* device) {
        // For now, a simple 1-to-1 allocation. No aliasing.
        // A real implementation would analyze resource lifetimes here to enable aliasing.
        m_physical_textures.resize(m_resources.size());

        for (size_t i = 0; i < m_resources.size(); ++i) {
            // Check if the resource is needed by a non-culled pass.
            bool is_needed = false;
            if (m_resources[i].producer_pass_idx != -1 && !m_passes[m_resources[i].producer_pass_idx]->m_is_culled) {
                is_needed = true;
            } else {
                // Check if it's read by any non-culled pass
                // (omitted for brevity, but necessary for imported resources)
            }

            if (is_needed) {
                // It's a texture resource (in a real engine, check the type)
                if (auto texture_desc = std::get_if<TextureDesc>(&m_resources[i].desc)) {
                    m_physical_textures[i] = device->create_texture(*texture_desc);
                } else if (auto buffer_desc = std::get_if<BufferDesc>(&m_resources[i].desc)) {
                    // If it's a buffer, we would allocate it differently.
                    // For now, we only handle textures in this example.
                    RDE_CORE_WARN("Buffer resources are not yet supported in RenderGraph allocation.");
                }else{
                    RDE_CORE_WARN("Resource at index {} is not known and will not be allocated.", i);
                }
            }
        }
    }

    void RenderGraph::record_pass_commands(ICommandBuffer& command_buffer, const RenderPacket& packet) {
        // This is the final step: iterate and execute.
        for (RGPass* pass : m_culled_passes) {
            // --- THIS IS THE CRITICAL PART ---
            // Before executing the pass, the RenderGraph is responsible for:
            // 1. Setting render targets: Bind the physical resources for pass->m_writes
            // 2. Setting up barriers: Ensure writes from previous passes are visible
            // 3. Setting viewport/scissor

            // Example (pseudo-code):
            // std::vector<GpuTextureHandle> render_targets;
            // for (auto& write_handle : pass->m_writes) {
            //     render_targets.push_back(m_physical_textures[write_handle.index]);
            // }
            // command_buffer.begin_render_pass(render_targets);

            // Now, call the user's execute lambda. They just do the drawing.
            pass->execute(command_buffer, packet);

            // command_buffer.end_render_pass();
        }
    }
}