// RDE_Project/modules/platform/glfw/GlfwWindow.h

#pragma once

#include "Core/Window.h"
#include <GLFW/glfw3.h>

class GlfwWindow : public Window {
public:
    GlfwWindow(const WindowProps &props);

    virtual ~GlfwWindow();

    void OnUpdate() override;

    unsigned int GetWidth() const override { return m_data.width; }
    unsigned int GetHeight() const override { return m_data.height; }

    void set_event_callback(const EventCallbackFn &callback) override { m_data.event_callback = callback; }

    void set_vsync(bool enabled) override;

    bool is_vsync() const override;

    virtual void *GetNativeWindow() const override { return m_window; }

private:
    virtual void init(const WindowProps &props);

    virtual void shutdown();

    GLFWwindow *m_window;

    struct WindowData {
        std::string title;
        unsigned int width, height;
        bool vsync;
        EventCallbackFn event_callback;
    };

    WindowData m_data;
};
