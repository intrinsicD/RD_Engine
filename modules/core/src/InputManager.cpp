#include "core/InputManager.h"
#include "core/Log.h"

namespace RDE {
    struct InputManager::InputState {
        // === Data Structures for the Switchboard ===

        // 1. The Action Map: Maps a name to the actual function to call.
        // This is the "logic" side.
        struct Action {
            ActionCallback callback;
            // You could add more here later, like a bool to check if it's enabled.
        };

        std::unordered_map<std::string, Action> actions;


        // 2. The Binding Maps: Maps a physical input to an action name and trigger type.
        // This is the "physical input" side.
        struct Binding {
            std::string action_name;
            InputTrigger trigger;
        };

        // We can have a list of bindings per key, e.g., for press AND release.
        std::unordered_map<KeyCode, std::vector<Binding> > key_bindings;
        std::unordered_map<MouseButton, std::vector<Binding> > mouse_bindings;
        // Future: std::unordered_map<GamepadButton, Binding> gamepad_bindings;


        // === Data Structures for State Tracking (from before) ===
        // This is needed for both the static query API and for detecting triggers.
        std::vector<bool> keys_current_frame;
        std::vector<bool> keys_last_frame;

        std::vector<bool> mouse_buttons_current_frame;
        std::vector<bool> mouse_buttons_last_frame;

        // ... other state like mouse position, scroll deltas, etc. ...
        CursorInfo cursor_info;
        DragInfo drag_info;
        ScrollInfo scroll_info;
        // This is used to track the last position for delta calculations
        [[nodiscard]] glm::vec2 get_last_position() const {
            return cursor_info.current_position - cursor_info.delta_from_last_frame;
            // Returns the last position before the current delta
        }
    };

    InputManager *InputManager::s_instance = nullptr;

    InputManager::InputManager() {
        s_instance = this;
        m_state = std::make_unique<InputState>();

        // Initialize state vectors to the correct size
        m_state->keys_current_frame.resize(KeyCode::KEY_LAST + 1, false);
        m_state->keys_last_frame.resize(KeyCode::KEY_LAST + 1, false);
        m_state->mouse_buttons_current_frame.resize(MouseButton::BUTTON_LAST + 1, false);
        m_state->mouse_buttons_last_frame.resize(MouseButton::BUTTON_LAST + 1, false);
    }

    InputManager::~InputManager() {
        s_instance = nullptr;
    }

    void InputManager::register_action(const std::string &action_name, ActionCallback callback) {
        // Simply store the callback in the action map, keyed by its name.
        m_state->actions[action_name] = {std::move(callback)};
    }

    void InputManager::bind_key_to_action(KeyCode key, InputTrigger trigger, const std::string &action_name) {
        // Check if the action name actually exists. This is good for debugging.
        if (m_state->actions.find(action_name) == m_state->actions.end()) {
            RDE_CORE_WARN("InputManager: Attempted to bind key to unregistered action '{}'", action_name);
            return;
        }
        // Add the binding to the list for that key.
        m_state->key_bindings[key].push_back({action_name, trigger});
    }

    void InputManager::bind_mouse_to_action(MouseButton button, InputTrigger trigger, const std::string &action_name) {
        if (m_state->actions.find(action_name) == m_state->actions.end()) {
            RDE_CORE_WARN("InputManager: Attempted to bind mouse button to unregistered action '{}'", action_name);
            return;
        }
        m_state->mouse_bindings[button].push_back({action_name, trigger});
    }

    // In InputManager.cpp

    void InputManager::on_event(Event &e) {
        EventDispatcher dispatcher(e);

        // Update internal state first (for polling)
        dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent &ev) {
            if (ev.is_repeat()) return false; // Ignore repeats for action bindings

            m_state->keys_current_frame[ev.get_key_code()] = true;

            if (auto it = m_state->key_bindings.find(ev.get_key_code()); it != m_state->key_bindings.end()) {
                for (const auto &binding: it->second) {
                    if (binding.trigger == InputTrigger::OnPress) {
                        if (auto action_it = m_state->actions.find(binding.action_name);
                            action_it != m_state->actions.end()) {
                            action_it->second.callback(0.0f);
                        }
                    }
                }
            }
            return false; // Don't consume the event
        });
        dispatcher.dispatch<KeyReleasedEvent>([this](KeyReleasedEvent &e) {
            m_state->keys_current_frame[e.get_key_code()] = false;
            return false;
        });

        // ... similar dispatchers for mouse buttons ...
        dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e) {
            m_state->mouse_buttons_current_frame[e.get_mouse_button()] = true;
            return false;
        });
        dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent &e) {
            m_state->mouse_buttons_current_frame[e.get_mouse_button()] = false;
            return false;
        });
        dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &e) {
            glm::vec2 last_position = m_state->cursor_info.current_position;
            m_state->cursor_info.current_position = glm::vec2(e.get_x(), e.get_y());
            m_state->cursor_info.delta_from_last_frame = m_state->cursor_info.current_position - last_position;
            return false; // Allow layers to handle the event
        });
        dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &e) {
            m_state->scroll_info.delta_this_frame = glm::vec2(e.get_x_offset(), e.get_y_offset());
            m_state->scroll_info.accumulated_offset += m_state->scroll_info.delta_this_frame;
            // Accumulate scroll offsets
            return false; // Allow layers to handle the event
        });

        // Handle drag events if needed
    }

    void InputManager::on_frame_end() {
        // Reset the current frame state to prepare for the next frame
        m_state->keys_last_frame = m_state->keys_current_frame;
        m_state->mouse_buttons_last_frame = m_state->mouse_buttons_current_frame;

        // Reset deltas
        m_state->scroll_info.delta_this_frame = {0.0f, 0.0f};

        // Clear current frame states
        std::fill(m_state->keys_current_frame.begin(), m_state->keys_current_frame.end(), false);
        std::fill(m_state->mouse_buttons_current_frame.begin(), m_state->mouse_buttons_current_frame.end(), false);
    }

    void InputManager::process_held_actions(float delta_time) {
        // --- Process Held Keys ---
        // Iterate only through the keys that have bindings. This is much faster.
        for (const auto &[key_code, bindings]: m_state->key_bindings) {
            // Is this key currently held down?
            if (m_state->keys_current_frame[key_code]) {
                for (const auto &binding: bindings) {
                    if (binding.trigger == InputTrigger::IsHeld) {
                        // Find the action and fire the callback
                        if (auto action_it = m_state->actions.find(binding.action_name);
                            action_it != m_state->actions.end()) {
                            action_it->second.callback(delta_time);
                        }
                    }
                }
            }
        }

        // --- Process Held Mouse Buttons ---
        // Do the same for mouse buttons
        for (const auto &[button, bindings]: m_state->mouse_bindings) {
            if (m_state->mouse_buttons_current_frame[button]) {
                for (const auto &binding: bindings) {
                    if (binding.trigger == InputTrigger::IsHeld) {
                        if (auto action_it = m_state->actions.find(binding.action_name);
                            action_it != m_state->actions.end()) {
                            action_it->second.callback(delta_time);
                        }
                    }
                }
            }
        }
    }

    bool InputManager::is_key_pressed(KeyCode key) {
        return s_instance && s_instance->m_state->keys_current_frame[key];
    }

    bool InputManager::is_key_released(KeyCode key) {
        return s_instance && !s_instance->m_state->keys_current_frame[key] && s_instance->m_state->keys_last_frame[key];
    }

    bool InputManager::is_key_held(KeyCode key) {
        return s_instance && s_instance->m_state->keys_current_frame[key] && s_instance->m_state->keys_last_frame[key];
    }

    bool InputManager::is_any_key_held() {
        return s_instance && std::any_of(s_instance->m_state->keys_current_frame.begin(),
                                         s_instance->m_state->keys_current_frame.end(),
                                         [](bool held) { return held; });
    }

    bool InputManager::is_mouse_button_pressed(MouseButton button) {
        return s_instance && s_instance->m_state->mouse_buttons_current_frame[button];
    }

    bool InputManager::is_mouse_button_released(MouseButton button) {
        return s_instance && !s_instance->m_state->mouse_buttons_current_frame[button] && s_instance->m_state->
               mouse_buttons_last_frame[button];
    }

    bool InputManager::is_mouse_button_held(MouseButton button) {
        return s_instance && s_instance->m_state->mouse_buttons_current_frame[button] && s_instance->m_state->
               mouse_buttons_last_frame[button];
    }

    bool InputManager::is_any_mouse_button_held() {
        return s_instance && std::any_of(s_instance->m_state->mouse_buttons_current_frame.begin(),
                                         s_instance->m_state->mouse_buttons_current_frame.end(),
                                         [](bool held) { return held; });
    }

    bool InputManager::is_mouse_scrolling() {
        return s_instance && s_instance->m_state->cursor_info.delta_from_last_frame != glm::vec2(0.0f, 0.0f);
    }

    bool InputManager::is_mouse_moving() {
        return s_instance && s_instance->m_state->cursor_info.delta_from_last_frame != glm::vec2(0.0f, 0.0f);
    }

    std::optional<CursorInfo> InputManager::get_cursor_info() {
        if (!s_instance) return std::nullopt;

        return std::optional<CursorInfo>{s_instance->m_state->cursor_info};
    }

    std::optional<DragInfo> InputManager::get_drag_info(MouseButton button) {
        if (!s_instance) return std::nullopt;

        // Check if the button has drag info
        if (s_instance->m_state->drag_info.button == button) {
            return std::optional<DragInfo>{s_instance->m_state->drag_info};
        }
        return std::nullopt; // No drag info for this button
    }

    std::optional<ScrollInfo> InputManager::get_scroll_info() {
        if (!s_instance) return std::nullopt;

        return std::optional<ScrollInfo>{s_instance->m_state->scroll_info};
    }
}
