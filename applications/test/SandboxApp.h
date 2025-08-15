#pragma once

#include "core/Application.h"
#include "core/IWindow.h"
#include "core/InputManager.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "material/MaterialDatabase.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"

namespace RDE {
    class ImGuiLayer;

    class EditorLayer;

    class SandboxApp : public Application {
    public:
        explicit SandboxApp(std::unique_ptr<IWindow> window = nullptr);

        ~SandboxApp() override;

        void run() override;

        entt::entity get_last_selected_entity() const { return m_last_selected_entity; }

        void set_last_selected_entity(entt::entity e) { m_last_selected_entity = e; }

    private:
        bool init() override;

        void shutdown() override;

        void on_update(float delta_time) override;

        void on_render() override;

        void on_event(Event &e) override;

        void ensure_primary_camera();


        void attach_editor_layer();

        std::unique_ptr<RDE::IWindow> m_window;
        std::unique_ptr<RDE::InputManager> m_input_manager;
        std::unique_ptr<RDE::Renderer> m_renderer;
        std::unique_ptr<RDE::AssetManager> m_asset_manager;
        std::unique_ptr<RDE::FileWatcher> m_file_watcher;
        std::unique_ptr<RDE::ThreadSafeQueue<std::string>> m_file_watcher_event_queue;

        // --- Data Ownership ---
        std::shared_ptr<RDE::AssetDatabase> m_asset_database;
        std::shared_ptr<RDE::MaterialDatabase> m_material_database;
        std::unique_ptr<RDE::Scene> m_scene;
        RDE::LayerStack m_layer_stack;
        ImGuiLayer *m_imgui_layer = nullptr; // Pointer to ImGui layer for UI rendering
        EditorLayer *m_editor_layer = nullptr; // Optional editor layer pointer

        // --- Application State ---
        bool m_is_running = true;
        bool m_is_minimized = false;
        bool m_window_resized = false;

        // --- Default Camera Config ---
        struct DefaultCameraConfig {
            CameraProjectionParameters projection{}; // perspective by default
            TransformLocal transform{{0.0f, 0.0f, 5.0f}, glm::quat(1, 0, 0, 0), {1, 1, 1}};
        };
        DefaultCameraConfig m_default_camera_config{};

        // --- Scene/Editor State ---
        entt::entity m_primary_camera_entity;
        entt::entity m_last_selected_entity;
        std::vector<entt::entity> m_selected_entities;

        RDE::View m_main_view;
    };
}
