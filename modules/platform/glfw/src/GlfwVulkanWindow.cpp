#include "GlfwVulkanWindow.h"
#include "Log.h"

#include <GLFW/glfw3.h>

namespace RDE{

    // Some static helper for initializing GLFW once
    static void InitializeGLFW() {
        static bool is_initialized = false;
        if (!is_initialized) {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW!");
            }
            // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            is_initialized = true;
        }
    }

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    std::unique_ptr<IWindow> IWindow::Create(const WindowConfig &config) {
        return std::make_unique<GlfwVulkanWindow>(config);
    }

    GlfwVulkanWindow::GlfwVulkanWindow(const WindowConfig &window_config) {
        m_data.title = window_config.title;
        m_data.width = window_config.width;
        m_data.height = window_config.height;

        InitializeGLFW();
        glfwSetErrorCallback(GlfwErrorCallback);

        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window!");
        }
    }

    GlfwVulkanWindow::~GlfwVulkanWindow() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        // Could add logic to terminate GLFW when the last window is destroyed.
    }

    void GlfwVulkanWindow::poll_events() {
        glfwPollEvents();
    }

    bool GlfwVulkanWindow::should_close() {
        return glfwWindowShouldClose(m_window);
    }
}