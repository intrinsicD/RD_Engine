// RDE_Project/modules/platform/glfw/GlfwWindow.h

#pragma once

#include "IWindow.h"

struct GLFWwindow;

namespace RDE {
    class GlfwVulkanWindow : public IWindow {
    public:
        explicit GlfwVulkanWindow(const WindowConfig &window_config = WindowConfig());

        ~GlfwVulkanWindow() override;

        void poll_events() override;

        unsigned int get_width() const override { return m_data.width; }

        unsigned int get_height() const override { return m_data.height; }

        void set_event_callback(const EventCallbackFn &callback) override { m_data.event_callback = callback; }

        void *get_native_handle() const override { return m_window; }

        bool should_close() override;

    private:

        void shutdown();

        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            EventCallbackFn event_callback;
        };

        WindowData m_data;
    };
}
