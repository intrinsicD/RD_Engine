#pragma once

#include "core/Application.h"
#include "core/IWindow.h"
#include "core/InputManager.h"
#include "core/FileWatcher.h"
#include "core/LayerStack.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "material/MaterialDatabase.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "assets/AssetHandle.h"
#include "core/SelectionContext.h"
#include "core/PrimaryCameraContext.h"

namespace RDE {
    class ImGuiLayer;

    class EditorLayer;

    class SandboxApp : public Application {
    public:
        explicit SandboxApp(std::unique_ptr<IWindow> window = nullptr);

        ~SandboxApp() override;

        void run() override;

        const SelectionContext& get_selection_context() const {
            return m_selection_context;
        }

        SelectionContext &get_selection_context() {
            return m_selection_context;
        }

        // NEW: window accessor for layers needing sizes/scales
        IWindow* get_window() const { return m_window.get(); }
    private:
        bool init() override;

        void shutdown() override;

        void on_update(float delta_time) override;

        void on_render() override;

        void on_event(Event &e) override;

        void ensure_primary_camera();

        // Create a scene entity from a loaded asset with sensible default components
        entt::entity instantiate_entity_from_asset(const AssetID &asset_id, const std::string &absolute_uri);

        void attach_editor_layer();

        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<InputManager> m_input_manager;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<FileWatcher> m_file_watcher;
        std::unique_ptr<ThreadSafeQueue<std::string>> m_file_watcher_event_queue;

        // --- Data Ownership ---
        std::shared_ptr<AssetDatabase> m_asset_database;
        std::shared_ptr<MaterialDatabase> m_material_database;
        std::unique_ptr<Scene> m_scene;
        LayerStack m_layer_stack;
        ImGuiLayer *m_imgui_layer = nullptr; // Pointer to ImGui layer for UI rendering
        EditorLayer *m_editor_layer = nullptr; // Optional editor layer pointer

        // --- Application State ---
        bool m_is_running = true;
        bool m_is_minimized = false;
        bool m_window_resized = false;

        // --- Scene/Editor State ---

        PrimaryCameraContext m_primary_camera_context;

        SelectionContext m_selection_context;

        View m_main_view;

        // --- Cached defaults ---
        AssetID m_default_material_asset; // default material for new renderables
    };
}
