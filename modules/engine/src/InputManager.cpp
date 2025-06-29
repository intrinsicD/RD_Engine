#include "InputManager.h"
#include "IWindow.h"
#include "../../log/include/Log.h"
#include "Input.h"

namespace RDE{
    bool InputManager::init() {
        // Initialize input handling here, e.g., set up GLFW callbacks if using GLFW.
        if(!window) {
            RDE_CORE_ERROR("InputManager: Window is not initialized!");
            return false;
        }
        return true; // Return true if initialization is successful.
    }

    void ResetMouseThisFrame(Mouse &mouse) {
        mouse.is_moving_this_frame = false;
        mouse.is_scrolling_this_frame = false;
        mouse.scroll_delta_xy = {0.0f, 0.0f};

        for (auto &button : mouse.button) {
            button.pressed_this_frame = false;
            button.released_this_frame = false;
        }
    }
    
    void SetMouseThisFrame(Mouse &mouse) {
        glm::vec2 prev_position = mouse.position;
        mouse.position = Input::GetMousePosition();
        mouse.delta = mouse.position - prev_position;

        mouse.is_moving_this_frame = (glm::length(mouse.delta) > 0.001f);
        const std::array<int, 3> button_mappings = {
                RDE_MOUSE_BUTTON_LEFT, RDE_MOUSE_BUTTON_RIGHT, RDE_MOUSE_BUTTON_MIDDLE
        };

        for (int i = 0; i < button_mappings.size(); ++i) {
            auto& button_state = mouse.button[i];
            bool was_pressed_last_frame = button_state.is_pressed;

            // Poll and update raw down state
            button_state.is_pressed = Input::IsMouseButtonPressed(button_mappings[i]);

            // Update transient flags based on state changes
            button_state.pressed_this_frame = !was_pressed_last_frame && button_state.is_pressed;
            button_state.released_this_frame = was_pressed_last_frame && !button_state.is_pressed;

            // Update dragging state (per button)
            if (button_state.pressed_this_frame) {
                mouse.is_dragging_this_frame = false; // Reset drag state on new press
                button_state.press_position = mouse.position;
            }
            if (button_state.released_this_frame) {
                mouse.is_dragging_this_frame = false;
                button_state.release_position = mouse.position;
            }

            // If a button is held and the mouse is moving, it's a drag operation.
            if (button_state.is_pressed && mouse.is_moving_this_frame) {
                mouse.is_dragging_this_frame = true;
            }
        }
    }

    void InputManager::process_input() {
        // Process input events here, e.g., polling events from the window.
        // Clear mouse state and keyboard state before processing new events.

        ResetMouseThisFrame(mouse);

        if(!m_event_queue.empty()){
            throw std::runtime_error("InputManager: Event queue is not empty, this should not happen!");
        }
        if(window) {
            window->poll_events();
        }

        SetMouseThisFrame(mouse);

        //TODO Keyboard processing
    }

    bool InputManager::on_mouse_scroll_event(MouseScrolledEvent &event){
        mouse.scroll_delta_xy = {event.get_x_offset(), event.get_y_offset()};
        mouse.is_scrolling_this_frame = true;
        return false;
    }
}