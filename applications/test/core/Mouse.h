#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace RDE{
    struct Mouse {
        struct Button {
            int button = 0; // GLFW mouse button code
            bool is_pressed = false; // Whether the button is currently pressed
            bool is_released = false; // Whether the button is currently released
            glm::vec2 press_position = {0.0f, 0.0f}; // Position of the mouse when the button was pressed
            glm::vec2 release_position = {0.0f, 0.0f}; // Position of the mouse when the button was released
        };
        std::vector<Button> buttons_current_frame;
        std::vector<Button> buttons_last_frame;

        glm::vec2 cursor_position = {0.0f, 0.0f}; // Current mouse position
        glm::vec2 delta_position = {0.0f, 0.0f}; // Change in mouse position since last frame
        glm::vec2 scroll_offset = {0.0f, 0.0f}; // Scroll offset for mouse wheel
        glm::vec2 scroll_delta = {0.0f, 0.0f}; // Scroll offset for mouse wheel

        bool is_dragging = false; // Flag to indicate if the mouse is currently dragging
        bool is_moving = false; // Flag to indicate if the mouse is currently moving

        bool any_pressed() const {
            for (const auto &button: buttons_current_frame) {
                if (button.is_pressed) {
                    return true; // Return true if any button is pressed
                }
            }
            return false; // No buttons are pressed
        }

        bool is_scrolling() const {
            return scroll_delta.x != 0.0f || scroll_delta.y != 0.0f;
        }

        [[nodiscard]] glm::vec2 get_last_position() const {
            return cursor_position - delta_position; // Returns the last position before the current delta
        }
    };
}