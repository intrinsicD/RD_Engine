#pragma once

#include "RenderGraphPass.h"
#include "RenderGraphResource.h"
#include "RenderPacket.h"

#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace RDE {

// Forward declarations
    class ICommandBuffer;

    class IGraphicsDevice;

    struct TextureDesc;

// Represents a virtual resource within the graph. It tracks its descriptor,
// producer pass, and usage throughout the graph.
    struct RGVirtualResource {
        std::string name; // For debugging
        // Union of descriptors for different resource types

        std::variant<TextureDesc, BufferDesc> desc; // The actual GPU resource handle

        // The pass that writes to this resource. -1 if it's an imported resource.
        int32_t producer_pass_idx = -1;
        uint32_t ref_count = 0;
    };


    class RenderGraph {
    public:
        RenderGraph() = default;

        // Disallow copy/move to prevent slicing and complex lifetime issues.
        RenderGraph(const RenderGraph &) = delete;

        RenderGraph &operator=(const RenderGraph &) = delete;

        // The main entry point for adding a pass to the graph.
        // The setup function is called immediately to declare dependencies.
        // The execute function is stored and called later during `execute()`.
        void add_pass(std::string_view name, RGPass::SetupFunc setup, RGPass::ExecuteFunc execute);

        // Sets an external, pre-existing resource as an "import" into the graph.
        // This is how you connect the graph to resources like the final swapchain image.
        // RGResourceHandle import_texture(std::string_view name, GpuTextureHandle texture);

        // Compiles the graph and executes the contained render passes.
        // This is the main function that orchestrates the entire rendering process for the frame.
        void execute(ICommandBuffer &command_buffer, const RenderPacket &packet, IGraphicsDevice *device);

    private:
        friend class RGBuilder; // Allows RGBuilder to call private methods like create_virtual_texture.

        // Internal method for creating a new virtual resource, called by RGBuilder.
        RGResourceHandle create_virtual_texture(const TextureDesc &desc);

        // --- Private methods for the compilation and execution phases ---

        // Step 1: Traverse graph from outputs to find which passes are actually used.
        void cull_passes();

        // Step 2: Allocate physical GPU resources for the virtual resources.
        void allocate_physical_resources(IGraphicsDevice *device);

        // Step 3: Iterate through the compiled passes and record their commands.
        void record_pass_commands(ICommandBuffer &command_buffer, const RenderPacket &packet);

        // Private member data
        std::string m_name;
        std::vector<std::unique_ptr<RGPass>> m_passes;
        std::vector<RGVirtualResource> m_resources;

        // A mapping from virtual resource handles to the actual physical handles.
        // This is populated during allocate_physical_resources().
        std::vector<GpuTextureHandle> m_physical_textures;

        // List of passes in the order they should be executed.
        std::vector<RGPass *> m_culled_passes;
    };
}