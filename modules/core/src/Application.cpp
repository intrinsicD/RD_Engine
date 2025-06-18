// RDE_Project/modules/core/src/Application.cpp

#include "Core/Application.h"
#include "Core/Log.h"

// Include specific event types we will create
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"

// Simple error callback for GLFW
static void GlfwErrorCallback(int error, const char *description) {
    RDE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

static Application *s_instance;

Application::Application(std::unique_ptr<Window> window): m_window(std::move(window)) {
    RDE_CORE_ASSERT(!s_instance, "Application already exists!");
    s_instance = this;

    m_window->set_event_callback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    m_imgui_layer = new ImGuiLayer();
    PushOverlay(m_imgui_layer);
}

Application::~Application() {
    RDE_CORE_INFO("Shutting down application.");
}

void Application::PushLayer(Layer *layer) {
    m_layer_stack.PushLayer(layer);
}

void Application::PushOverlay(Layer *layer) {
    m_layer_stack.PushOverlay(layer);
}

Application &Application::Get() { return *s_instance; }

void Application::Run() {
    while (m_is_running) {
        m_imgui_layer->Begin(); // New
        for (Layer *layer: m_layer_stack)
            layer->OnUpdate();
        m_imgui_layer->End(); // New

        m_window->OnUpdate();
    }
}

void Application::OnEvent(Event &e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));

    // Iterate backwards so overlays get events first.
    for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it) {
        if (e.handled)
            break;
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent &e) {
    m_is_running = false;
    return true; // The event is handled.
}
