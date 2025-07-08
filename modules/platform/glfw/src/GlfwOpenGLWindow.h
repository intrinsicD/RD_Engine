// RDE_Project/modules/platform/glfw/GlfwWindow.h

#pragma once

#include "core/IWindow.h"

struct GLFWwindow;

namespace RDE {
    class GlfwOpenGLWindow : public IWindow {
    public:
        explicit GlfwOpenGLWindow(const WindowConfig &window_config = WindowConfig());

        ~GlfwOpenGLWindow() override;

        void set_event_callback(const EventCallbackFn &callback) override { m_data.event_callback = callback; }

        void poll_events() override;

        bool should_close() override;

        void swap_buffers() override;

        [[nodiscard]] const char *get_title() const override { return m_data.title.c_str(); }

        [[nodiscard]] int get_width() const override { return m_data.width; }

        [[nodiscard]] int get_height() const override { return m_data.height; }

        [[nodiscard]] void *get_native_window() const override { return m_window; }

        void set_vsync(bool enabled) override;

        [[nodiscard]] bool is_vsync() const override { return m_data.vsync; }

    private:
        void shutdown();

        GLFWwindow *m_window = nullptr;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            bool vsync;
            EventCallbackFn event_callback;
        };

        WindowData m_data;
    };
}
