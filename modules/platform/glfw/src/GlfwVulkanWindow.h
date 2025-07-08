// RDE_Project/modules/platform/glfw/GlfwWindow.h

#pragma once

#include "core/IWindow.h"

struct GLFWwindow;

namespace RDE {
    class GlfwVulkanWindow : public IWindow {
    public:
        explicit GlfwVulkanWindow(const WindowConfig &window_config = WindowConfig());

        ~GlfwVulkanWindow() override;

        void set_event_callback(const EventCallbackFn &callback) override { m_data.event_callback = callback; }

        void poll_events() override;

        bool should_close() override;

        [[nodiscard]] int get_width() const override { return m_data.width; }

        [[nodiscard]] int get_height() const override { return m_data.height; }

        [[nodiscard]] void *get_native_handle() const override { return m_window; }

    private:

        void shutdown();

        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            int width, height;
            EventCallbackFn event_callback;
        };

        WindowData m_data;
    };
}
