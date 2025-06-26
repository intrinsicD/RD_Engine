#pragma once

namespace RDE {
    class Scene;

    struct FrameContext {
        float delta_time = 0.0f;
        float fixed_time_step = 0.0f;
        float total_time = 0.0f;
        bool is_minimized = false;
        Scene *scene = nullptr;
    };
}