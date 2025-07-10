#pragma once

#include "core/Application.h"
#include "core/IWindow.h"
#include "core/InputManager.h"
#include "renderer/Renderer.h"

namespace RDE {
    class ImGuiLayer;

    class SandboxApp : public Application {
    public:
        explicit SandboxApp(std::unique_ptr<IWindow> window = nullptr);

        ~SandboxApp() override;

        void run(int width, int height, const char *title) override;

    private:
        bool init(int width, int height, const char *title) override;

        void shutdown() override;

        void on_update(float delta_time) override;

        void on_render() override;

        void on_event(Event &e) override;

        std::unique_ptr<RDE::IWindow> m_window;
        std::unique_ptr<RDE::InputManager> m_input_manager;
        std::unique_ptr<RDE::Renderer> m_renderer;
        std::unique_ptr<RDE::AssetManager> m_asset_manager;
        std::unique_ptr<RDE::FileWatcher> m_file_watcher;
        std::unique_ptr<RDE::ThreadSafeQueue<std::string>> m_file_watcher_event_queue;
        std::unique_ptr<RDE::SystemScheduler> m_system_scheduler;

        // --- Data Ownership ---
        std::shared_ptr<RDE::AssetDatabase> m_asset_database;
        std::shared_ptr<entt::registry> m_registry;
        std::shared_ptr<entt::dispatcher> m_dispatcher;
        RDE::LayerStack m_layer_stack;
        ImGuiLayer *m_imgui_layer = nullptr; // Pointer to ImGui layer for UI rendering

        // --- Application State ---
        bool m_is_running = true;
        bool m_is_minimized = false;
        bool m_window_resized = false;

        // --- Scene/Editor State ---
        entt::entity m_primary_camera_entity;
        entt::entity m_last_selected_entity;
        std::vector<entt::entity> m_selected_entities;
    };
}
