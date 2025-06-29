#include "systems/CullingSystem.h"
#include "../../../log/include/Log.h"

namespace RDE{
    void CullingSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("CullingSystem attached");
    }

    void CullingSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("CullingSystem detached");
    }

    void CullingSystem::on_update(Scene *scene, float delta_time) {
        // Update camera logic here
    }
}