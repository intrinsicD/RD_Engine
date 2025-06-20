#pragma once

#include "ISystem.h"

namespace RDE {
    class Scene; // Forward declare
    class Event;

    class RenderSystem : public ISystem {
    public:
        RenderSystem() = default;

        ~RenderSystem() override = default;

        void on_attach(Scene *scene) override;

        void on_pre_update(Scene *scene, float delta_time) override;

        void on_update(Scene *scene, float delta_time) override;

        void on_post_update(Scene *scene, float delta_time) override;

        void on_event(Scene *scene, Event &e) override;
    };
}
