#pragma once

#include "ISystem.h"
#include <memory>

namespace RDE {
    class Scene; // Forward declare
    class Event;
    class Entity;
    class IRenderer;

    class RenderSystem : public ISystem {
    public:
        RenderSystem() = default;

        ~RenderSystem() override = default;

        void on_attach(Scene *scene) override;

        void on_pre_update(Scene *scene, float delta_time) override;

        void on_update(Scene *scene, float delta_time) override;

        void on_post_update(Scene *scene, float delta_time) override;

        void on_event(Scene *scene, Event &e) override;

        void set_renderer(Scene *scene, std::shared_ptr<IRenderer> renderer);

        Entity get_primary_camera(Scene *scene);
    };
}
