// RDE_Project/modules/core/include/Input.h
#pragma once

#include <glm/glm.hpp>

namespace RDE {
    class Input {
    public:
        static bool is_key_pressed(int keycode);

        static bool is_mouse_button_pressed(int button);

        static glm::vec2 get_mouse_position();

        static float get_mouse_x();

        static float get_mouse_y();
    };
}