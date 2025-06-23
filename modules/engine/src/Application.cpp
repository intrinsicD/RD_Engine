#include "Application.h"
#include "LayerStack.h"
#include "Base.h"
#include "layers/ImGuiLayer.h"

#include "src/GlfwOpenGLWindow.h"
#include "src/GlfwVulkanWindow.h"

namespace RDE {
    Application::Application(const Config::WindowConfig &window_config,
                             const Config::RendererConfig &renderer_config): m_window(GlfwOpenGLWindow::Create(
                                                                                 window_config)),
                                                                             m_renderer(OpenGLRenderer::Create()),
                                                                             m_asset_manager(
                                                                                 std::make_unique<AssetManager>()),
                                                                             m_is_running(true), m_is_minimized(false) {
        if (renderer_config.api == Config::RendererAPI::OpenGL_4_5) {
            m_window = GlfwOpenGLWindow::Create(window_config);
            m_renderer = OpenGLRenderer::Create(window_config, renderer_config);
        } else if (renderer_config.api == Config::RendererAPI::Vulkan_1_2) {
            m_window = GlfwVulkanWindow::Create(window_config);
            m_renderer = VulkanRenderer::Create(window_config, renderer_config);
        } else {
            // Default to OpenGL if no valid API is specified
            RDE_CORE_ASSERT(false, "Unknown RendererAPI");
        }

        auto imgui_layer = std::make_shared<ImGuiLayer>();
        m_imgui_layer = imgui_layer.get();
        push_overlay(imgui_layer);
    }

    Application::~Application() {
        m_layer_stack.reset();
        m_asset_manager.reset();
        m_renderer.reset();
        m_window.reset();
    }

    void Application::run() {
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
        dispatcher.dispatch<WindowCloseEvent>(RDE_BIND_EVENT_FN(Application::on_window_close));
        dispatcher.dispatch<WindowResizeEvent>(RDE_BIND_EVENT_FN(Application::on_window_resize));

        for (auto it = m_layer_stack->rbegin(); it != m_layer_stack->rend(); ++it) {
            if (e.handled) {
                break;
            }
            (*it)->on_event(e);
        }
    }

    ILayer *Application::push_layer(std::shared_ptr<ILayer> layer) {
        m_layer_stack->push_layer(layer);
        layer->on_attach();
        return layer.get();
    }

    ILayer *Application::push_overlay(std::shared_ptr<ILayer> overlay) {
        m_layer_stack->push_overlay(overlay);
        overlay->on_attach();
        return overlay.get();
    }

    Application &Application::get() {
        static Application instance;
        return instance;
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
