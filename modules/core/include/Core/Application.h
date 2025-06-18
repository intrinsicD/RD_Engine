// RDE_Project/modules/core/include/Core/Application.h

#pragma once

#include "Core/Base.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Core/Events/Event.h" // Include our new event header
#include "Core/Events/ApplicationEvent.h" // And the application-specific events
#include "Core/LayerStack.h" // Include LayerStack
#include "Core/ImGuiLayer.h" // Include LayerStack

class Application {
public:
    Application(std::unique_ptr<Window> window);

    virtual ~Application();

    // The main run loop for the application.
    void Run();

    // Non-copyable and non-movable
    Application(const Application &) = delete;

    Application &operator=(const Application &) = delete;

    Application(Application &&) = delete;

    Application &operator=(Application &&) = delete;

    // The main event handling function, to be overridden by clients.
    virtual void OnEvent(Event &e);

    void PushLayer(Layer *layer);

    void PushOverlay(Layer *layer);

    static Application &Get();

    Window &get_window() const { return *m_window; }

private:
    bool OnWindowClose(WindowCloseEvent &e);

    // This is the main window for the application.
    // NOTE-DESIGN: In a more complex engine, this would be a unique_ptr to a
    // custom Window class that wraps the GLFWwindow. For now, we keep it simple.
    std::unique_ptr<Window> m_window;
    bool m_is_running = true;
    LayerStack m_layer_stack;
    ImGuiLayer *m_imgui_layer;
};

// This function must be defined in a client project (e.g., Sandbox)
// and will return a new instance of their specific application class.
Application *CreateApplication();
