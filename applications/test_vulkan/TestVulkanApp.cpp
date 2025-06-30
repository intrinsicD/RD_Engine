#include "src/GlfwVulkanWindow.h"
#include "VulkanRenderer.h"
#include <iostream>
#include <memory>

class Application {
public:
    void run() {
        // --- PHASE 1: SETUP ---
        std::cout << "Starting application...\n";
        RDE::WindowConfig window_config;
        window_config.title = "Helios Engine";
        window_config.width = 1280;
        window_config.height = 720;

        m_window = RDE::GlfwVulkanWindow::Create(window_config);
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
        while (!m_window->should_close()) {
            m_window->poll_events();
            m_renderer->draw_frame();
        }

        // --- PHASE 4: CLEANUP ---
        // RAII handles most of this, but we must wait for the GPU to be idle
        // before the destructors for the renderer and window run.
        m_renderer->get_device()->wait_idle();
        std::cout << "Application shutting down.\n";
    }

private:
    void prepare_resources(){}; //TODO

    std::unique_ptr<RDE::IWindow> m_window;
    std::unique_ptr<RDE::IRenderer> m_renderer;

    // Handles to our triangle's assets
    RAL::BufferHandle m_triangle_vertex_buffer;
    RAL::PipelineHandle m_triangle_pipeline;
};

int main() {
    Application app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}