// RDE_Project/modules/core/src/Application.cpp

#include "Application.h"
#include "Log.h"

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

        auto imgui_layer = std::make_unique<ImGuiLayer>();
        m_imgui_layer = imgui_layer.get();
        push_overlay(std::move(imgui_layer));
    }

    Application::~Application() {
        RDE_CORE_INFO("Shutting down application.");
    }

    Layer * Application::push_layer(std::unique_ptr<Layer> layer) {
        return m_layer_stack.push_layer(std::move(layer));
    }

    Layer * Application::push_overlay(std::unique_ptr<Layer> overlay) {
        return m_layer_stack.push_overlay(std::move(overlay));
    }

    Application &Application::get() { return *s_instance; }

    void Application::run() {
        while (m_is_running) {
            for (const auto &layer: m_layer_stack)
                layer->on_update();

            m_imgui_layer->begin();
            for (const auto &layer: m_layer_stack) {
                layer->on_gui_render();
            }
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