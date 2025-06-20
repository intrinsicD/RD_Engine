
#include "InputSystem.h"
#include "Scene.h"
#include "Input.h"
#include "InputCodes.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "ContextComponents/MouseContextComponent.h"

namespace RDE {
    void InputSystem::on_attach(Scene *scene) {
        scene->get_registry().ctx().emplace<MouseContextComponent>();
    }

    void InputSystem::on_update(Scene *scene, float delta_time) {
        auto &context = scene->get_registry().ctx().get<MouseContextComponent>();

        // --- Store previous frame's position ---
        context.prev_position = context.current_position;

        // --- Poll current frame's RAW state ---
        context.current_position = Input::GetMousePosition();
        context.position_delta = context.current_position - context.prev_position;
        context.is_moving = (glm::length(context.position_delta) > 0.001f);

        // Map our enum to your low-level input codes
        const std::array<int, 3> button_mappings = { RDE_MOUSE_BUTTON_LEFT, RDE_MOUSE_BUTTON_RIGHT, RDE_MOUSE_BUTTON_MIDDLE };

        // --- Process each button in a loop ---
        for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i) {
            MouseButtonState& button_state = context.buttons[i];
            bool was_down = button_state.is_down;

            // Poll and update raw down state
            button_state.is_down = Input::IsMouseButtonPressed(button_mappings[i]);

            // Update transient flags
            button_state.pressed_this_frame = !was_down && button_state.is_down;
            button_state.released_this_frame = was_down && !button_state.is_down;

            // Update dragging logic
            if (button_state.pressed_this_frame) {
                button_state.is_dragging = false; // Reset drag state on new press
                button_state.press_position = context.current_position;
            } else if (button_state.is_down && context.is_moving) {
                // NOTE: A drag only starts if the mouse moves *after* the press.
                // A sensitivity threshold is often used here.
                if (glm::distance(button_state.press_position, context.current_position) > 2.0f) {
                    button_state.is_dragging = true;
                }
            }

            if (button_state.released_this_frame) {
                button_state.is_dragging = false;
            }
        }
    }

    void InputSystem::on_post_update(Scene *scene, float delta_time) {
        auto &context = scene->get_registry().ctx().get<MouseContextComponent>();

        // Reset all transient, "this frame only" flags.
        for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i) {
            MouseButtonState& button_state = context.buttons[i];
            button_state.pressed_this_frame = false;
            button_state.released_this_frame = false;
        }

        context.is_scrolling = false;
        context.scroll_delta_xy = {0.0f, 0.0f};
    }

    void InputSystem::on_event(Scene *scene, Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>([&](MouseScrolledEvent &event) -> bool {
            auto &context = scene->get_registry().ctx().get<MouseContextComponent>();
            context.scroll_delta_xy = {event.get_x_offset(), event.get_y_offset()};
            context.is_scrolling = true;
            return false;
        });
    }
}