#include "systems/CameraSystem.h"
#include "../../../log/include/Log.h"

namespace RDE{
    void CameraSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("CameraSystem attached");
    }

    void CameraSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("CameraSystem detached");
    }

    void CameraSystem::on_update(Scene *scene, float delta_time) {
        // Update camera logic here
    }
}