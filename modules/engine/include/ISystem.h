#pragma once

namespace RDE {
    class Event;
    class Scene;

    // An interface for all systems that operate on the scene.
    // This provides a consistent structure for engine and application logic.
    class ISystem {
    public:
        virtual ~ISystem() = default;

        // Called once when the system is attached to the application/layer.
        virtual void on_attach(Scene *scene) {}

        // Called once when the system is detached.
        virtual void on_detach(Scene *scene) {}

        // Optional: Called before the main update loop for all systems.
        virtual void on_pre_update(Scene *scene, float delta_time) {}

        // Called every frame to perform the system's logic.
        virtual void on_update(Scene *scene, float delta_time) = 0;

        // Optional: Called after the main update loop for all systems.
        virtual void on_post_update(Scene *scene, float delta_time) {}

        // Called to allow the system to process an event.
        virtual void on_event(Scene *scene, Event &e) {}
    };
}