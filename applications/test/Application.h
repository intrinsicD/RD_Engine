#pragma once

#include "events/Event.h"

#include "AssetManager.h"
#include "AssetDatabase.h"
#include "FileWatcher.h"

#include <glm/glm.hpp>
#include <entt/fwd.hpp>
#include <set>

struct GLFWwindow;

namespace RDE {
    struct Mouse {
        struct Button {
            int button = 0; // GLFW mouse button code
            bool is_pressed = false; // Whether the button is currently pressed
            bool is_released = false; // Whether the button is currently released
            glm::vec2 press_position = {0.0f, 0.0f}; // Position of the mouse when the button was pressed
            glm::vec2 release_position = {0.0f, 0.0f}; // Position of the mouse when the button was released
        };

        std::vector<Button> buttons_current_frame{3};
        std::vector<Button> buttons_last_frame{3};

        glm::vec2 cursor_position = {0.0f, 0.0f}; // Current mouse position
        glm::vec2 delta_position = {0.0f, 0.0f}; // Change in mouse position since last frame
        glm::vec2 scroll_offset = {0.0f, 0.0f}; // Scroll offset for mouse wheel
        glm::vec2 scroll_delta = {0.0f, 0.0f}; // Scroll offset for mouse wheel

        bool is_dragging = false; // Flag to indicate if the mouse is currently dragging
        bool is_moving = false; // Flag to indicate if the mouse is currently moving

        bool any_pressed() const {
            for (const auto &button: buttons_current_frame) {
                if (button.is_pressed) {
                    return true; // Return true if any button is pressed
                }
            }
            return false; // No buttons are pressed
        }

        bool is_scrolling() const {
            return scroll_delta.x != 0.0f || scroll_delta.y != 0.0f;
        }

        [[nodiscard]] glm::vec2 get_last_position() const {
            return cursor_position - delta_position; // Returns the last position before the current delta
        }
    };

    struct Keyboard {
        std::set<int> keys_held_this_frame; // Set of keys currently held down
        std::vector<bool> keys_pressed_current_frame = std::vector<bool>(1024, false);
        // List of keys released this frame
        std::vector<bool> keys_pressed_last_frame = std::vector<bool>(1024, false); ;
        // List of keys released this frame
        std::vector<bool> keys_repeated = std::vector<bool>(1024, false); ; // List of keys released this frame
    };

    struct ApplicationContext;

    class ILayer {
    public:
        virtual ~ILayer() = default;

        virtual void on_attach(const ApplicationContext &app_context) = 0;

        virtual void on_detach(const ApplicationContext &app_context) = 0;

        virtual void on_update(const ApplicationContext &app_context) = 0;

        virtual void on_render(const ApplicationContext &app_context) = 0;

        virtual void on_render_gui(const ApplicationContext &app_context) = 0;

        virtual void on_event(Event &e, const ApplicationContext &app_context) = 0;

        virtual const std::string &get_name() const = 0;
    };

    class LayerStack {
    public:
        LayerStack() = default;

        ~LayerStack() = default;

        ILayer *push_layer(std::shared_ptr<ILayer> layer, ApplicationContext &app_context) {
            auto it = m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer));
            m_layer_insert_index++;
            (*it)->on_attach(app_context); // Call the attach hook.
            return it->get();
        }

        ILayer *push_overlay(std::shared_ptr<ILayer> overlay, ApplicationContext &app_context) {
            // Overlays are always added to the very end of the list.
            m_layers.emplace_back(std::move(overlay));
            m_layers.back()->on_attach(app_context); // Call the attach hook.
            return m_layers.back().get();
        }

        void pop_layer(ILayer *layer, ApplicationContext &app_context) {
            auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layer_insert_index,
                                   [layer](const std::shared_ptr<ILayer> &l) { return l.get() == layer; });

            if (it != m_layers.begin() + m_layer_insert_index) {
                (*it)->on_detach(app_context);
                m_layers.erase(it);
                m_layer_insert_index--; // Decrement the boundary index as a normal layer was removed.
            }
        }

        void pop_overlay(ILayer *overlay, ApplicationContext &app_context) {
            auto it = std::find_if(m_layers.begin() + m_layer_insert_index, m_layers.end(),
                                   [overlay](const std::shared_ptr<ILayer> &l) { return l.get() == overlay; });

            if (it != m_layers.end()) {
                (*it)->on_detach(app_context);
                m_layers.erase(it); // Simply remove the overlay. The insert index is not affected.
            }
        }

        std::vector<std::shared_ptr<ILayer> >::iterator begin() { return m_layers.begin(); }

        std::vector<std::shared_ptr<ILayer> >::iterator end() { return m_layers.end(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rbegin() { return m_layers.rbegin(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rend() { return m_layers.rend(); }

    private:
        std::vector<std::shared_ptr<ILayer> > m_layers;
        unsigned int m_layer_insert_index = 0;
    };

    struct ApplicationContext {
        GLFWwindow *m_window = nullptr; // Pointer to the GLFW window
        std::string m_title; // Title of the application window
        int m_width = 0;
        int m_height = 0;
        bool m_is_running = true; // Flag to control the main loop
        bool m_is_minimized = false; // Flag to check if the window is minimized

        std::shared_ptr<entt::registry> m_registry; // Entity registry for managing entities and components
        std::shared_ptr<entt::dispatcher> m_dispatcher; // Event dispatcher for handling events

        std::shared_ptr<AssetDatabase> m_asset_database;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<FileWatcher> m_file_watcher; // File watcher for monitoring asset changes
        std::unique_ptr<ThreadSafeQueue<std::string>> m_file_watcher_event_queue; // File watcher for monitoring asset changes

        entt::entity m_primary_camera_entity; // Entity representing the camera in the scene
        entt::entity m_last_selected_entity; // Entity currently selected by the user

        std::vector<entt::entity> m_selected_entities; // List of entities in the scene
        std::function<void(Event &)> m_event_callback; // Callback function for handling events

        Mouse m_mouse_state; // Input state for mouse
        Keyboard m_keyboard_state; // Input state for keyboard

        std::unordered_map<int, std::function<void()> > m_key_press_bindings; // Key bindings for press actions
        std::unordered_map<int, std::function<void()> > m_key_release_bindings; // Key bindings for release actions
        std::unordered_map<int, std::function<void()> > m_key_repeat_bindings; // Key bindings for release actions
        std::unordered_map<int, std::function<void()> > m_key_update_bindings; // Key bindings for update actions

        LayerStack m_layer_stack; // Stack of layers for the application
    };

    class Application {
    public:
        Application();

        ~Application();

        void run(int width, int height, const char *title);

        ApplicationContext &get_app_context() {
            return *m_app_context;
        }

    private:
        bool init(int width, int height, const char *title);

        void shutdown();

        void on_update();

        void on_render();

        void on_render_gui();

        void on_event(Event &e);

        std::shared_ptr<ApplicationContext> m_app_context; // Application context containing the main window and state
    };
}
