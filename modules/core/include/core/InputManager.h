// core/InputManager.h

#pragma once

#include "core/KeyCodes.h"
#include "core/MouseCodes.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

#include <glm/glm.hpp>
#include <functional>
#include <optional>
#include <string>
#include <memory> // For std::unique_ptr

namespace RDE {
    // --- Data Structs for Queries (Unchanged - They are excellent) ---
    struct CursorInfo {
        glm::vec2 current_position;
        glm::vec2 delta_from_last_frame;
    };

    struct DragInfo {
        MouseButton button;
        glm::vec2 start_position;
        glm::vec2 current_position;
        glm::vec2 delta_from_last_frame;
    };

    // Note: Simplified ScrollInfo. Direction is implicit in delta.y.
    struct ScrollInfo {
        glm::vec2 accumulated_offset; // Accumulated scroll offset
        glm::vec2 delta_this_frame;
    };

    // --- Action Binding Types ---
    enum class InputTrigger {
        OnPress, // Fires once when input transitions to active
        OnRelease, // Fires once when input transitions to inactive
        IsHeld // Fires every frame the input is active
    };

    // Callback can optionally receive delta time for frame-rate independent logic
    using ActionCallback = std::function<void(float /*delta_time*/)>;

    class InputManager {
    public:
        InputManager();

        ~InputManager();

        // --- Core Lifecycle ---
        // Consumes raw engine events from the application event queue
        void on_event(Event &e);

        // Processes held-action bindings, called once per frame in the main loop
        void process_held_actions(float delta_time);

        // Resets per-frame state (e.g., deltas, pressed/released flags). Call at frame end.
        void on_frame_end();


        // --- Unified Action Mapping API ---
        void register_action(const std::string& action_name, ActionCallback callback);

        // Step 2: Bind a physical input to trigger a registered action.
        // This can be done by default config, or later by a settings screen.
        void bind_key_to_action(KeyCode key, InputTrigger trigger, const std::string& action_name);

        void bind_mouse_to_action(MouseButton button, InputTrigger trigger, const std::string& action_name);

        // Future: void bind_gamepad_action(...)


        // --- Static Query API ---
        static bool is_key_pressed(KeyCode key);

        static bool is_key_released(KeyCode key);

        static bool is_key_held(KeyCode key);

        static bool is_any_key_held();

        static bool is_mouse_button_pressed(MouseButton button);

        static bool is_mouse_button_released(MouseButton button);

        static bool is_mouse_button_held(MouseButton button);

        static bool is_any_mouse_button_held();

        static bool is_mouse_moving();

        static bool is_mouse_scrolling();

        static CursorInfo get_cursor_info();

        static std::optional<DragInfo> get_drag_info(MouseButton button);

        static ScrollInfo get_scroll_info();

    private:
        // --- PIMPL (Pointer to Implementation) ---
        // All internal state (key buffers, mouse state, binding maps) is now
        // hidden inside this struct, which is defined ONLY in the .cpp file.
        // This decouples the interface from the implementation.
        struct InputState;
        std::unique_ptr<InputState> m_state;

        // The singleton instance used by the static query functions.
        // It is owned and managed by the Application class.
        static InputManager *s_instance;
    };
}
