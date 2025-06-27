#pragma once

#include <vector>
#include <memory>

// Forward declarations
namespace RDE {
    class Scene;
    class IRenderer;
    class IRenderableCollector;
    struct RenderPacket;

    class IRenderPipeline {
    public:
        virtual ~IRenderPipeline() = default;

        // Step 1: Tell the scene HOW to collect data for this frame.
        // The pipeline provides the list of collectors it needs.
        virtual void collect(Scene *scene, RenderPacket &render_packet) = 0;

        // Step 2: Take the collected data and execute the render.
        // The pipeline builds its specific RenderGraph inside this call.
        virtual void render(IRenderer *renderer, const RenderPacket &packet) = 0;
    };

}