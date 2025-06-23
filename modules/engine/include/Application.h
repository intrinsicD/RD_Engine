// RDE_Project/modules/core/include/Application.h

#pragma once

#include "Log.h"
#include "events/Event.h"
#include "events/ApplicationEvent.h"
#include "ApplicationConfig.h"

namespace RDE {
    class IWindow;
    class IRenderer;
    class AssetManager;
    class LayerStack;
    class ILayer;
    class ImGuiLayer;


    class Application {
    public:
        explicit Application(const Config::WindowConfig &window_config = {},
                             const Config::RendererConfig &renderer_config = {});

        virtual ~Application();

        void run();

        Application(const Application &) = delete;

        Application &operator=(const Application &) = delete;

        Application(Application &&) = delete;

        Application &operator=(Application &&) = delete;

        virtual void on_event(Event &e);

        ILayer *push_layer(std::shared_ptr<ILayer> layer);

        ILayer *push_overlay(std::shared_ptr<ILayer> overlay);

        static Application &get();

        IWindow &get_window() const { return *m_window; }

        IRenderer &get_renderer() const { return *m_renderer; }

        AssetManager &get_asset_manager() const { return *m_asset_manager; }

    protected:
        bool on_window_close(WindowCloseEvent &e);

        bool on_window_resize(WindowResizeEvent &e);

        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IRenderer> m_renderer;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<LayerStack> m_layer_stack;
        ImGuiLayer *m_imgui_layer;

        bool m_is_running = true;
        bool m_is_minimized = false;
    };

    // Free function, CamelCase
    Application *CreateApplication();
}
