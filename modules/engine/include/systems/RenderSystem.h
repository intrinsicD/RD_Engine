#pragma once

#include "ISystem.h"
#include "../../../renderer/include/RenderPacket.h"

namespace RDE {
    class RenderSystem : public ISystem {
    public:
        RenderSystem() = default;

        ~RenderSystem() override = default;

        void on_attach(Scene *scene) override;

        void on_detach(Scene *scene) override;

        void on_pre_update(Scene *scene, float delta_time) override;

        void on_update(Scene *scene, float delta_time) override;

        RenderPacket collect_renderables(Scene *scene);
    };
}
