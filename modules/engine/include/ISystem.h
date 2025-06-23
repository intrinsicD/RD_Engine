#pragma once

namespace RDE {
    class Event;

    // An interface for all systems that operate on the scene.
    // This provides a consistent structure for engine and application logic.
    class ISystem {
    public:
        virtual ~ISystem() = default;

        // Called once when the system is attached to the application/layer.
        virtual void on_attach() {}

        // Called once when the system is detached.
        virtual void on_detach() {}

        // Optional: Called before the main update loop for all systems.
        virtual void on_pre_update(float delta_time) {}

        // Called every frame to perform the system's logic.
        virtual void on_update(float delta_time) = 0;

        // Optional: Called after the main update loop for all systems.
        virtual void on_post_update(float delta_time) {}

        // Called to allow the system to process an event.
        virtual void on_event(Event &e) {}
    };
}