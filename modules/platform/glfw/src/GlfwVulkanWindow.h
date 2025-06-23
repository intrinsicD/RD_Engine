// RDE_Project/modules/platform/glfw/GlfwWindow.h

#pragma once

#include "IWindow.h"

struct GLFWwindow;

namespace RDE {
    class GlfwVulkanWindow : public IWindow {
    public:
        GlfwVulkanWindow(const Config::WindowConfig &window_config,
                   const Config::RendererConfig &renderer_config = Config::RendererConfig());

        ~GlfwVulkanWindow() override;

        void on_update() override;

        unsigned int get_width() const override { return m_data.width; }

        unsigned int get_height() const override { return m_data.height; }

        void set_event_callback(const EventCallbackFn &callback) override { m_data.event_callback = callback; }

        void set_vsync(bool enabled) override;

        bool is_vsync() const override;

        void *get_native_window() const override { return m_window; }

    private:
        void init(const Config::WindowConfig &window_config, const Config::RendererConfig &renderer_config);

        void init_opengl();

        void init_vulkan();

        void shutdown();

        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            bool vsync;
            EventCallbackFn event_callback;
        };

        WindowData m_data;
    };
}
