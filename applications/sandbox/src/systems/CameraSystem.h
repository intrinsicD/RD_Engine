#pragma once

#include "ISystem.h"

namespace RDE {
    class Scene; // Forward declare
    class Event;
    class Entity;

    class CameraSystem : public ISystem {
    public:
        // Constructor
        CameraSystem() = default;

        ~CameraSystem() override = default;

        // Called once when the system is attached to the scene
        void on_attach(Scene *scene) override;

        // Called every frame to perform camera updates
        void on_update(Scene *scene, float delta_time) override;

        // Called to process events related to camera controls
        void on_event(Scene *scene, Event &e) override;

        // Get the view-projection matrix for the primary camera
        Entity get_primary_camera_entity(Scene *scene) const;
    };
}
