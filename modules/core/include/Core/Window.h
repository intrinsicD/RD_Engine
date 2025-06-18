// RDE_Project/modules/core/include/Core/Window.h

#pragma once

#include "Core/Events/Event.h"
#include <string>
#include <functional>
#include <memory>

struct WindowProps {
    std::string title;
    unsigned int width;
    unsigned int height;

    WindowProps(const std::string &p_title = "RD_Engine",
                unsigned int p_width = 1280,
                unsigned int p_height = 720)
        : title(p_title), width(p_width), height(p_height) {
    }
};

// Interface representing a desktop system based Window
class Window {
public:
    using EventCallbackFn = std::function<void(Event &)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual unsigned int GetWidth() const = 0;

    virtual unsigned int GetHeight() const = 0;

    // Window attributes
    virtual void set_event_callback(const EventCallbackFn &callback) = 0;

    virtual void set_vsync(bool enabled) = 0;

    virtual bool is_vsync() const = 0;

    virtual void *GetNativeWindow() const = 0;

    // A factory method to create the appropriate window based on the platform.
    // For now, we only have a GLFW implementation.
    static std::unique_ptr<Window> Create(const WindowProps &props = WindowProps());
};
