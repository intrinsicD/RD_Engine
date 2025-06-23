#pragma once
#include "AssetManager.h"

namespace RDE::Components {
    struct AnimationComponent {
        AssetHandle animation_handle;
        float current_time = 0.0f; // Current time in the animation
        float playback_speed = 1.0f; // Speed of playback, can be negative for reverse playback
        bool is_looping = false; // Whether the animation should loop
    };
}
