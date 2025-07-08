#pragma once

#include <vector>
#include <set>

namespace RDE {
    struct Keyboard {
        std::set<int> keys_held_this_frame; // Set of keys currently held down
        std::vector<bool> keys_pressed_current_frame; // List of keys released this frame
        std::vector<bool> keys_pressed_last_frame; // List of keys released this frame
        std::vector<bool> keys_repeated; // List of keys released this frame
    };
}