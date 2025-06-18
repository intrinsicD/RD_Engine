// RDE_Project/modules/core/src/Application.cpp

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
namespace RDE {
// Free function: CamelCase
    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    static Application *s_instance = nullptr;

    Application::Application(std::unique_ptr<Window> window)
            : m_window(std::move(window)) {
        RDE_CORE_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;

        m_window->set_event_callback(std::bind(&Application::on_event, this, std::placeholders::_1));

        m_imgui_layer = new ImGuiLayer();
        push_overlay(m_imgui_layer);
    }

    Application::~Application() {
        RDE_CORE_INFO("Shutting down application.");
    }

    void Application::push_layer(Layer *layer) {
        m_layer_stack.push_layer(layer);
    }

    void Application::push_overlay(Layer *layer) {
        m_layer_stack.push_overlay(layer);
    }

    Application &Application::get() { return *s_instance; }

    void Application::run() {
        while (m_is_running) {
            m_imgui_layer->begin();
            for (Layer *layer: m_layer_stack)
                layer->on_update();
            m_imgui_layer->end();

            m_window->on_update();
        }
    }

    void Application::on_event(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::on_window_close, this, std::placeholders::_1));

        for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it) {
            if (e.handled)
                break;
            (*it)->on_event(e);
        }
    }

    bool Application::on_window_close(WindowCloseEvent &e) {
        m_is_running = false;
        return true;
    }
}