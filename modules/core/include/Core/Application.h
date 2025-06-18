// RDE_Project/modules/core/include/Core/Application.h

#pragma once

#include "Core/Base.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Core/Events/Event.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/LayerStack.h"
#include "Core/ImGuiLayer.h"

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