#include "systems/AnimationSystem.h"
#include "Log.h"

namespace RDE {
    void AnimationSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("AnimationSystem attached");
    }

    void AnimationSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("AnimationSystem detached");
    }

    void AnimationSystem::on_update(Scene *scene, float delta_time) {

    }
}