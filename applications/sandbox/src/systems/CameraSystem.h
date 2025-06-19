#pragma once

#include <glm/glm.hpp>

namespace RDE {
    class Scene; // Forward declare
    class Event;

    namespace CameraSystem {
        // This system updates all arcball-controlled cameras
        void UpdateArcball(Scene *scene, float delta_time);

        // This system handles events for arcball-controlled cameras
        void OnEventArcball(Scene *scene, Event &e);

        // A helper to get the main camera's view-projection matrix
        glm::mat4 GetViewProjection(Scene *scene);
    }
}
