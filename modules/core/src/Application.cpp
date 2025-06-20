// RDE_Project/modules/core/src/Application.cpp

#include "Application.h"
#include "Log.h"

#include <chrono>

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

        m_window->set_event_callback(BIND_EVENT_FN(Application::on_event));

        auto imgui_layer = std::make_unique<ImGuiLayer>();
        m_imgui_layer = imgui_layer.get();
        push_overlay(std::move(imgui_layer));
    }

    Application::~Application() {
        RDE_CORE_INFO("Shutting down application.");
    }

    Layer *Application::push_layer(std::shared_ptr<Layer> layer) {
        return m_layer_stack.push_layer(layer);
    }

    Layer *Application::push_overlay(std::shared_ptr<Layer> overlay) {
        return m_layer_stack.push_overlay(overlay);
    }

    Application &Application::get() { return *s_instance; }

    void Application::run() {
        //Timer

        auto start_time = std::chrono::high_resolution_clock::now();
        while (m_is_running) {
            // Calculate delta time
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> delta_time = current_time - start_time;
            start_time = current_time;

            for (const auto &layer: m_layer_stack)
                layer->on_update(delta_time.count());

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
        dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::on_window_close));
        dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::on_window_resize));

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

    bool Application::on_window_resize(WindowResizeEvent &e) {
        // Don't process if minimized
        if (e.get_width() == 0 || e.get_height() == 0) {
            m_is_minimized = true;
            return false;
        }
        m_is_minimized = false;

        // This function's only job is to manage the minimized state.
        // It MUST return false to allow layers to handle the event.
        return false;
    }
}