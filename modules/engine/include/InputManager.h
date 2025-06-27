#pragma once

#include "IWindow.h"
#include "events/MouseEvent.h"

#include <vector>
#include <glm/glm.hpp>

namespace RDE {
    struct Mouse {
        struct ButtonState {
            bool pressed_this_frame = false;
            bool is_pressed = false;
            bool released_this_frame = false;

            glm::vec2 press_position = {0.0f, 0.0f}; // Position when the button was pressed.
            glm::vec2 release_position = {0.0f, 0.0f};  // Position when the button was released.
        };

        glm::vec2 position = {0.0f, 0.0f}; // Current mouse position.
        glm::vec2 delta = {0.0f, 0.0f}; // Change in position since the last frame.

        ButtonState button[3];

        bool is_moving_this_frame = false;
        bool is_dragging_this_frame = false;
        bool is_scrolling_this_frame = false;
        glm::vec2 scroll_delta_xy = {0.0f, 0.0f};

        glm::vec2 get_prev_position_this_frame() const {
            return position - delta;
        }
    };

    class InputManager {
    public:
        explicit InputManager(IWindow *window) : window(window) {};

        ~InputManager() = default;

        // Initializes the input manager with the window handle.
        bool init();

        // Processes input events.
        void process_input();

        std::vector<Event> &fetch_events() {
            return m_event_queue;
        }

        bool on_mouse_scroll_event(MouseScrolledEvent &event);

        const Mouse &get_mouse() const {
            return mouse;
        }

    private:
        IWindow *window;
        Mouse mouse; // Mouse state.

        std::vector<Event> m_event_queue; // Queue to store input events.
    };
}