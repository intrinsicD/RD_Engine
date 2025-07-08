#pragma once

#include "core/events/Event.h"
#include "core/Mouse.h"
#include "core/Keyboard.h"
#include "scene/SystemScheduler.h"
#include "core/LayerStack.h"
#include "core/IWindow.h"

#include "AssetManager.h"
#include "AssetDatabase.h"
#include "FileWatcher.h"

#include <entt/fwd.hpp>

struct GLFWwindow;

namespace RDE {
    struct ApplicationContext {
        //TODO descide where to put IWindow...
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

        Mouse m_mouse_state;// Input state for mouse
        Keyboard m_keyboard_state; // Input state for keyboard

        std::unordered_map<int, std::function<void()>> m_key_press_bindings; // Key bindings for press actions
        std::unordered_map<int, std::function<void()>> m_key_release_bindings; // Key bindings for release actions
        std::unordered_map<int, std::function<void()>> m_key_repeat_bindings; // Key bindings for release actions
        std::unordered_map<int, std::function<void()>> m_key_update_bindings; // Key bindings for update actions

        LayerStack m_layer_stack; // Stack of layers for the application
        std::unique_ptr<SystemScheduler> m_system_scheduler; // System scheduler for managing systems and their execution order
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

        void on_update(float delta_time);

        void on_render();

        void on_render_gui();

        void on_event(Event &e);

        std::shared_ptr<ApplicationContext> m_app_context; // Application context containing the main window and state
    };
}