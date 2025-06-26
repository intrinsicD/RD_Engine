#pragma once

#include "RenderGraphResource.h"

#include <functional>
#include <string>

namespace RDE {
    class RGBuilder;
    class ICommandBuffer; // Forward declaration of the command buffer interface
    class RenderPacket; // Forward declaration of the command buffer interface

    // A single node in the RenderGraph
    class RGPass {
    public:
        using SetupFunc = std::function<void(RGBuilder &)>;
        using ExecuteFunc = std::function<void(ICommandBuffer &, const RenderPacket &)>;

        RGPass(std::string name, SetupFunc setup, ExecuteFunc execute)
            : m_name(std::move(name)), m_setup(std::move(setup)), m_execute(std::move(execute)) {
        }

        void execute(ICommandBuffer &cmd, const RenderPacket &packet) { m_execute(cmd, packet); }

        // Friend classes to access private members
        friend class RenderGraph;
        friend class RGBuilder;

    private:
        std::string m_name;
        SetupFunc m_setup;
        ExecuteFunc m_execute;

        std::vector<RGResourceHandle> m_reads;
        std::vector<RGResourceHandle> m_writes;

        uint32_t m_ref_count = 0;
    };
}
