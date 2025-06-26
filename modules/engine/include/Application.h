// RDE_Project/modules/core/include/Application.h

#pragma once

#include "Log.h"
#include "Engine.h"
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
    class JobSystem;

    class Application {
    public:
        explicit Application(std::unique_ptr<IWindow> window, std::unique_ptr<IRenderer> renderer);

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

        Engine &get_engine() const { return *m_engine; }

    protected:
        virtual bool on_initialize();

        virtual void on_shutdown();

        std::unique_ptr<Engine> m_engine;
    };

    // Free function, CamelCase
    Application *CreateApplication();
}
