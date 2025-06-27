#include "pipelines/ForwardPipeline.h"
#include "IRenderer.h"
#include "graph/RenderGraph.h"
#include "passes/ForwardOpaquePass.h"
#include "passes/ImGuiPass.h"

namespace RDE{
    ForwardPipeline::ForwardPipeline() {
        // Initialize the pipeline with necessary resources, shaders, etc.
    }

    ForwardPipeline::~ForwardPipeline() {
        // Cleanup resources if needed
    }

    void ForwardPipeline::collect(Scene *scene, RenderPacket &render_packet) {
        // Collect data from the scene for rendering
        // This could involve gathering meshes, lights, cameras, etc.
        for (const auto& collector : m_collectors) {
            collector->collect(registry, packet);
        }
    }

    void ForwardPipeline::render(IRenderer *renderer, const RenderPacket &packet) {
        RenderGraph rg;

        // For a simple forward pipeline, our render target is the final swapchain image.
        // The RenderGraph needs a way to identify this. Let's assume the renderer provides it.
        RGResourceHandle backbuffer_handle = renderer->get_backbuffer_handle();

        // We also need a depth buffer. The renderer should manage this persistent resource.
        RGResourceHandle depth_buffer_handle = renderer->get_main_depth_buffer_handle();

        // 1. Add the main pass for drawing the opaque 3D scene.
        setup_forward_opaque_pass(rg, packet, backbuffer_handle, depth_buffer_handle);

        // 2. Add the UI pass, which draws on top of the same backbuffer.
        setup_imgui_pass(rg, backbuffer_handle);

        // Tell the renderer to execute this assembled graph.
        renderer->submit(rg);
    }
}