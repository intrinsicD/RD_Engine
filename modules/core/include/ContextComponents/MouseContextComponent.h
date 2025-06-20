#pragma once

#include <glm/glm.hpp>

namespace RDE{
    // Holds the complete input state for a single mouse button.
    struct MouseButtonState {
        bool is_down = false;
        bool pressed_this_frame = false;
        bool released_this_frame = false;
        bool is_dragging = false;
        glm::vec2 press_position{0.0f, 0.0f};
    };

    // An enum class is strongly-typed and safer.
    enum class MouseButton : int {
        Left = 0,
        Right = 1,
        Middle = 2,
        Count // A common trick to get the number of enum values
    };

    struct MouseContextComponent {
        // --- Core State ---
        glm::vec2 current_position{0.0f, 0.0f};
        glm::vec2 prev_position{0.0f, 0.0f}; // Correct place for this state
        glm::vec2 position_delta{0.0f, 0.0f};

        // --- Per-Button State ---
        // Use an array indexed by the MouseButton enum for clean, scalable access.
        MouseButtonState buttons[static_cast<int>(MouseButton::Count)];

        // --- Movement & Scroll ---
        bool is_moving = false;
        bool is_scrolling = false;
        glm::vec2 scroll_delta_xy{0.0f, 0.0f};
    };
}