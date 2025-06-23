#include "systems/InputSystem.h"
#include "components/MouseContext.h"
#include "Input.h"
#include "Base.h"
#include "Scene.h"
#include "events/MouseEvent.h"

#include <array>

namespace RDE {
    void InputSystem::on_attach(Scene *scene) {
        scene->get_context().emplace<Components::MouseContextComponent>();
    }

    void InputSystem::on_pre_update(Scene *scene, float delta_time) {
        auto &mouse_context = scene->get_context().get<Components::MouseContextComponent>();

        glm::vec2 prev_position = mouse_context.position;
        mouse_context.position = Input::GetMousePosition();
        mouse_context.delta = mouse_context.position - prev_position;

        mouse_context.is_moving_this_frame = (glm::length(mouse_context.delta) > 0.001f);
        const std::array<int, 3> button_mappings = {
            RDE_MOUSE_BUTTON_LEFT, RDE_MOUSE_BUTTON_RIGHT, RDE_MOUSE_BUTTON_MIDDLE
        };

        for (int i = 0; i < button_mappings.size(); ++i) {
            auto& button_state = mouse_context.buttons[i];
            bool was_pressed_last_frame = button_state.is_pressed;

            // Poll and update raw down state
            button_state.is_pressed = Input::IsMouseButtonPressed(button_mappings[i]);

            // Update transient flags based on state changes
            button_state.was_pressed_this_frame = !was_pressed_last_frame && button_state.is_pressed;
            button_state.was_released_this_frame = was_pressed_last_frame && !button_state.is_pressed;

            // Update dragging state (per button)
            if (button_state.was_pressed_this_frame) {
                button_state.is_dragging = false; // Reset drag state on new press
                button_state.press_position = mouse_context.position;
            }
            if (button_state.was_released_this_frame) {
                button_state.is_dragging = false;
                button_state.release_position = mouse_context.position;
            }

            // If a button is held and the mouse is moving, it's a drag operation.
            if (button_state.is_pressed && mouse_context.is_moving_this_frame) {
                button_state.is_dragging = true;
            }
        }
    }

    void InputSystem::on_post_update(Scene *scene, float delta_time) {
        auto &mouse_context = scene->get_context().get<Components::MouseContextComponent>();
        mouse_context.is_moving_this_frame = false;
        mouse_context.is_scrolling_this_frame = false;
        mouse_context.scroll_delta_xy = {0.0f, 0.0f};

        for (auto &button : mouse_context.button) {
            button.pressed_this_frame = false;
            button.released_this_frame = false;
        }
    }

    void InputSystem::on_event(Scene *scene, Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>([&](MouseScrolledEvent &event) -> bool {
            auto &context = scene->get_context().get<Components::MouseContextComponent>();
            context.scroll_delta_xy = {event.get_x_offset(), event.get_y_offset()};
            context.is_scrolling_this_frame = true;
            return false;
        });
    }
}
