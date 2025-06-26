#pragma once

#include "events/Event.h"

#include <vector>

namespace RDE{
    class InputManager {
    public:
        InputManager() = default;

        ~InputManager() = default;

        // Initializes the input manager with the window handle.
        bool init(void *window_handle);

        // Processes input events.
        void process_input();

        std::vector<Event> &fetch_events() {
            return m_event_queue;
        }

        // Checks if a key is pressed.
        bool is_key_pressed(int key) const;

        // Checks if a mouse button is pressed.
        bool is_mouse_button_pressed(int button) const;

        // Gets the current mouse position.
        void get_mouse_position(float &x, float &y) const;

    private:
        void *m_window_handle = nullptr; // Handle to the window for input events.
        std::vector<Event> m_event_queue; // Queue to store input events.
    };
}