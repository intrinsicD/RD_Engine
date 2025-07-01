#include "src/GlfwVulkanWindow.h"
#include "VulkanRenderer.h"
#include "Log.h"
#include "events/Event.h"
#include "events/ApplicationEvent.h"
#include <iostream>
#include <memory>

namespace RDE {

    class Application {
    public:
        void run() {
            RDE::Log::Initialize(); // Initialize the logging system
            // --- PHASE 1: SETUP ---
            std::cout << "Starting application...\n";
            RDE::WindowConfig window_config;
            window_config.title = "Helios Engine";
            window_config.width = 1280;
            window_config.height = 720;

            m_window = RDE::GlfwVulkanWindow::Create(window_config);
            m_window->set_event_callback(RDE_BIND_EVENT_FN(Application::on_event));
            m_renderer = std::make_unique<RDE::VulkanRenderer>();

            RAL::SwapchainDescription swapchain_desc;
            swapchain_desc.native_window_handle = m_window->get_native_handle();
            swapchain_desc.width = m_window->get_width();
            swapchain_desc.height = m_window->get_height();
            swapchain_desc.vsync = true;
            m_renderer->get_device()->create_swapchain(swapchain_desc);

            // --- PHASE 2: RESOURCE PREPARATION ---
            prepare_resources();

            // --- PHASE 3: MAIN LOOP ---
            while (m_is_running) {
                m_window->poll_events();
                if (m_is_minimized) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue; // Skip rendering if minimized
                }
                FrameContext context = m_renderer->begin_frame();

                if (context.is_valid) {
                    // --- Record Commands ---
                    // The application is now in full control.
                    // It can use a render graph, task system, or simple loops.
                    // It records all its commands into context.command_buffer.
                    //my_render_graph->execute(context.command_buffer);

                    // --- Submit to GPU ---
                    m_renderer->submit_and_present(context);
                }
            }

            // --- PHASE 4: CLEANUP ---
            // RAII handles most of this, but we must wait for the GPU to be idle
            // before the destructors for the renderer and window run.
            m_renderer->get_device()->wait_idle();
            std::cout << "Application shutting down.\n";
        }

    private:
        void prepare_resources() {}; //TODO

        void on_event(Event &e){
            EventDispatcher dispatcher(e);
            dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &) {
                m_is_running = false;
                return true;
            });

            dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) {
                // Don't process if minimized
                if (e.get_width() == 0 || e.get_height() == 0) {
                    m_is_minimized = true;
                    return false;
                }
                m_is_minimized = false;

                //m_renderer->on_window_resize(e.get_width(), e.get_height());
                // This function's only job is to manage the minimized state.
                // It MUST return false to allow layers to handle the event.
                return false;
            });
        }

        bool m_is_running = true;
        bool m_is_minimized = false;

        std::unique_ptr<RDE::IWindow> m_window;
        std::unique_ptr<RDE::IRenderer> m_renderer;

        // Handles to our triangle's assets
        RAL::BufferHandle m_triangle_vertex_buffer;
        RAL::PipelineHandle m_triangle_pipeline;
    };
}

int main() {
    RDE::Application app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}