#include "InputManager.h"
#include "IWindow.h"
#include "Log.h"
#include "Input.h"

namespace RDE {
    InputManager::InputManager(void *window_handle) : m_window_handle(window_handle) {
        if (!m_window_handle) {
            RDE_CORE_ERROR("InputManager initialized with a null window handle!");
        }
    }
    void InputManager::begin_frame() {
// --- 1. Reset transient flags from the PREVIOUS frame ---
        mouse.is_moving_this_frame = false;
        mouse.is_scrolling_this_frame = false;
        mouse.scroll_delta_xy = {0.0f, 0.0f};

        for (auto &button : mouse.button) {
            button.pressed_this_frame = false;
            button.released_this_frame = false;
        }

        // --- 2. Poll continuous state for the CURRENT frame ---
        glm::vec2 last_frame_position = mouse.position;

        // Assume IWindow has a static method to get the main window handle for polling.
        mouse.position = Input::GetMousePosition(m_window_handle);

        // Calculate delta based on this frame's poll vs last frame's poll.
        mouse.delta = mouse.position - last_frame_position;

        if (glm::length(mouse.delta) > 0.001f) {
            mouse.is_moving_this_frame = true;
        }

        // --- 3. Poll button states and derive complex state (like dragging) ---
        bool any_button_pressed = false;
        for (int i = 0; i < 3; ++i) {
            auto& button_state = mouse.button[i];
            bool was_pressed = button_state.is_pressed;

            // Poll for the raw "is down" state
            button_state.is_pressed = Input::IsMouseButtonPressed(m_window_handle, i);

            // Derive transient flags from the change in state
            button_state.pressed_this_frame = button_state.is_pressed && !was_pressed;
            button_state.released_this_frame = !button_state.is_pressed && was_pressed;

            if (button_state.pressed_this_frame) {
                button_state.press_position = mouse.position;
            }
            if (button_state.released_this_frame) {
                button_state.release_position = mouse.position;
            }

            if (button_state.is_pressed) {
                any_button_pressed = true;
            }
        }

        // Dragging is a global mouse state: it's true if ANY button
        // is held down AND the mouse is moving.
        mouse.is_dragging_this_frame = any_button_pressed && mouse.is_moving_this_frame;
    }

    void InputManager::end_frame() {

    }

    void InputManager::on_event(RDE::Event &e) {
        EventDispatcher dispatcher(e);
        // We only care about scroll events here now. All other mouse state
        // is derived from polling in begin_frame().
        dispatcher.dispatch<MouseScrolledEvent>(RDE_BIND_EVENT_FN(InputManager::on_mouse_scroll_event));
    }

    bool InputManager::on_mouse_scroll_event(MouseScrolledEvent &e) {
        mouse.scroll_delta_xy = {e.get_x_offset(), e.get_y_offset()};
        mouse.is_scrolling_this_frame = true;
        return false;
    }
}