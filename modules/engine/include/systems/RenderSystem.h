#pragma once

#include "ISystem.h"

namespace RDE {
    class RenderSystem : public ISystem {
    public:
        RenderSystem() = default;

        ~RenderSystem() override = default;

        void on_attach(Scene *scene) override;

        void on_detach(Scene *scene) override;

        void on_update(Scene *scene, float delta_time) override;
    };
}
