#pragma once

#include <string>

namespace RDE{
    class RenderGraph {
    public:
        virtual ~RenderGraph() = default;

        // --- PASS SUBMISSION ---
        // The main way to add work to the graph.
        virtual void add_pass(const std::string& name, std::function<void(RHI_CommandList&)>&& execute_lambda) = 0;

        // --- RESOURCE MANAGEMENT & BLACKBOARD ---
        // The blackboard is a key-value store for per-frame data.
        // This is how you pass the camera data.
        template<typename T>
        void set_global(const std::string& name, T&& data);

        template<typename T>
        T& get_global(const std::string& name);

        // Methods to declare dependencies (as discussed before)
        // .reads_from(resource_name)
        // .writes_to(resource_name)
    };
}