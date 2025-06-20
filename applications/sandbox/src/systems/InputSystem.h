#pragma once

#include "ISystem.h"

namespace RDE {
    class Scene; // Forward declare
    class Event;

    class InputSystem : public ISystem {
    public:
        // Constructor
        InputSystem() = default;

        ~InputSystem() override = default;

        // Called once when the system is attached to the scene
        void on_attach(Scene *scene) override;

        // Called every frame to perform camera updates
        void on_update(Scene *scene, float delta_time) override;

        void on_post_update(Scene *scene, float delta_time) override;

        // Called to process events related to camera controls
        void on_event(Scene *scene, Event &e) override;
    };
}
