// RDE_Project/modules/core/include/Application.h

#pragma once

#include "Log.h"
#include "../../core/include/Events/Event.h"
#include "../../core/include/Events/ApplicationEvent.h"
#include "../../core/include/LayerStack.h"
#include "../../core/include/ImGuiLayer.h"
#include "../../core/include/Window.h"
#include "../../core/include/Scene.h"
#include "AssetManager.h"

namespace RDE {
    class Renderer;

    class Application {
    public:
        explicit Application(std::unique_ptr<Window> window);

        virtual ~Application();

        void run();

        Application(const Application &) = delete;

        Application &operator=(const Application &) = delete;

        Application(Application &&) = delete;

        Application &operator=(Application &&) = delete;

        virtual void on_event(Event &e);

        Layer *push_layer(std::shared_ptr<Layer> layer);

        Layer *push_overlay(std::shared_ptr<Layer> overlay);

        static Application &get();

        Window &get_window() const { return *m_window; }

        Scene &get_scene() const { return *m_scene; }

        Renderer &get_renderer() const { return *m_renderer; }

        AssetManager &get_asset_manager() const { return *m_asset_manager; }

    protected:
        bool on_window_close(WindowCloseEvent &e);

        bool on_window_resize(WindowResizeEvent &e);

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Scene> m_scene;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<AssetManager> m_asset_manager;

        bool m_is_running = true;
        bool m_is_minimized = false;
        LayerStack m_layer_stack;
        ImGuiLayer *m_imgui_layer;
    };

// Free function, CamelCase
    Application *CreateApplication();
}