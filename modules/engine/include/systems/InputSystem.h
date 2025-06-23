#pragma once

#include "ISystem.h"

namespace RDE {
    class InputSystem : public ISystem{
    public:
        InputSystem() = default;

        ~InputSystem() override = default;

        void on_attach(Scene *scene) override;

        void on_pre_update(Scene *scene, float delta_time) override;

        void on_post_update(Scene *scene, float delta_time) override;

        void on_event(Scene *scene, Event &e) override;
    };
}