#pragma once
#include <memory>

// Forward declarations
namespace RDE {
    class IRenderPipeline;
    class IRenderer;
    class Scene;
}

namespace RDE {

    class RenderPipelineManager {
    public:
        RenderPipelineManager(std::unique_ptr<IRenderPipeline> initial_pipeline);

        // Sets a new rendering pipeline, allowing for runtime strategy changes.
        void set_pipeline(std::unique_ptr<IRenderPipeline> pipeline);

        // The main orchestration function for a frame.
        void execute_frame(Scene* scene, IRenderer* renderer);

    private:
        std::unique_ptr<IRenderPipeline> m_active_pipeline;
    };
}