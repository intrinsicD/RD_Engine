#include "systems/PhysicsSystem.h"
#include "Log.h"

namespace RDE{
    void PhysicsSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("PhysicsSystem attached");
    }

    void PhysicsSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("PhysicsSystem detached");
    }

    void PhysicsSystem::on_update(Scene *scene, float delta_time) {
        // Update camera logic here
    }
}