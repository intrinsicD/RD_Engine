// RDE_Project/modules/core/include/Core/Application.h

#pragma once

#include "Log.h"
#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "LayerStack.h"
#include "ImGuiLayer.h"

namespace RDE {


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

        Layer *push_layer(std::unique_ptr<Layer> layer);

        Layer *push_overlay(std::unique_ptr<Layer> overlay);

        static Application &get();

        Window &get_window() const { return *m_window; }

    private:
        bool on_window_close(WindowCloseEvent &e);

        std::unique_ptr<Window> m_window;
        bool m_is_running = true;
        LayerStack m_layer_stack;
        ImGuiLayer *m_imgui_layer;
    };

// Free function, CamelCase
    Application *CreateApplication();
}