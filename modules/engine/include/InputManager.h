#pragma once

#include "ApplicationContext.h"
#include "events/Event.h"

#include <vector>

namespace RDE{
    class InputManager {
    public:
        InputManager() = default;

        ~InputManager() = default;

        // Initializes the input manager with the window handle.
        bool init(ApplicationContext &context);

        // Processes input events.
        void process_input();

        std::vector<Event> &fetch_events() {
            return m_event_queue;
        }

        void on_event(Event &event);

    private:
        void *m_window_handle = nullptr; // Handle to the window for input events.

        std::vector<Event> m_event_queue; // Queue to store input events.
    };
}