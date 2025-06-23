#pragma once

#include <glm/glm.hpp>

namespace RDE::Components {
    struct MouseContextComponent {
        struct ButtonState {
            bool pressed_this_frame = false;
            bool is_pressed = false;
            bool released_this_frame = false;

            glm::vec2 press_position;
            glm::vec2 release_position;
        };

        glm::vec2 position;
        glm::vec2 delta;

        ButtonState button[3];

        bool is_moving_this_frame = false;
        bool is_dragging_this_frame = false;
        bool is_scrolling_this_frame = false;
        glm::vec2 scroll_delta_xy = {0.0f, 0.0f};

        glm::vec2 get_prev_position_this_frame() const {
            return position - delta;
        }
    };
}
