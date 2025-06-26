#pragma once

#include <string>
#include <memory>
#include <vector>

namespace RDE {
    // Forward declarations
    class RenderGraph;
    class RGBuilder;
    class RGPass;
    class RenderPacket;
    class VirtualResourceDesc;
    class IGraphicsDevice;
    class ICommandBuffer; // A key RAL interface we'll define later

    class RenderGraph {
    public:
        RenderGraph() = default;

        // The main entry point for adding a pass.
        // The setup function is called immediately to declare dependencies.
        void add_pass(std::string name, RGPass::SetupFunc setup, RGPass::ExecuteFunc execute);

        // The function that does all the magic.
        void execute(ICommandBuffer& cmd, const RenderPacket& packet, IGraphicsDevice* device);

    private:
        friend class RGBuilder; // Allows builder to access private graph data

        // ... internal data structures ...
        std::vector<std::unique_ptr<RGPass>> m_passes;
        // A list of virtual resource descriptors
        std::vector<VirtualResourceDesc> m_resources;
    };
}