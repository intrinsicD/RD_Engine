#include "RenderPipelineManager.h"
#include "IRenderPipeline.h"
#include "Scene.h"
#include "IRenderer.h"
#include "RenderPacket.h"

namespace RDE {
    RenderPipelineManager::RenderPipelineManager(std::unique_ptr<IRenderPipeline> initial_pipeline)
            : m_active_pipeline(std::move(initial_pipeline)) {}

    void RenderPipelineManager::set_pipeline(std::unique_ptr<IRenderPipeline> pipeline) {
        m_active_pipeline = std::move(pipeline);
    }

    void RenderPipelineManager::execute_frame(Scene* scene, IRenderer* renderer) {
        if (!m_active_pipeline || !scene || !renderer) {
            return; // Nothing to do
        }

        // --- Step 1: Data Collection ---
        // Create an empty packet to be filled.
        RenderPacket packet;
        // The manager tells the pipeline to populate the packet using data from the scene's registry.
        m_active_pipeline->collect(scene, packet);

        // --- Step 2: Rendering ---
        renderer->begin_frame();
        // The manager tells the pipeline to render the packet using the renderer.
        m_active_pipeline->render(renderer, packet);
        renderer->present_frame();
    }
}