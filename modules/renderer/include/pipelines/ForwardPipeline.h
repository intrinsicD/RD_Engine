#pragma once

#include "IRenderPipeline.h"

namespace RDE{
    class ForwardPipeline : public  IRenderPipeline {
    public:
        ForwardPipeline();

        ~ForwardPipeline() override;

        void collect(Scene *scene, RenderPacket &render_packet) override;

        void render(IRenderer *renderer, const RenderPacket &packet) override;
    private:
        // This pipeline owns the specific collectors it requires.
        std::vector<std::unique_ptr<IRenderableCollector>> m_collectors;
    };
}