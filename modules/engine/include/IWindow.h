// RDE_Project/modules/core/include/Window.h

#pragma once

#include "events/Event.h"
#include <string>
#include <functional>
#include <memory>


namespace RDE {
    struct WindowConfig {
        std::string title = "RD_Engine";
        int width = 1280;
        int height = 720;
    };
    // Interface representing a desktop system based Window
    class IWindow {
    public:
        using EventCallbackFn = std::function<void(Event &)>;

        virtual ~IWindow() = default;

        virtual bool init() = 0;

        virtual void poll_events() = 0;

        virtual void on_update() = 0;

        virtual unsigned int get_width() const = 0;

        virtual unsigned int get_height() const = 0;

        // Window attributes
        virtual void set_event_callback(const EventCallbackFn &callback) = 0;

        virtual void set_vsync(bool enabled) = 0;

        virtual bool is_vsync() const = 0;

        virtual void *get_native_window() const = 0;

        virtual void close() = 0;

        // A factory method to create the appropriate window based on the platform.
        // For now, we only have a GLFW implementation.
        static std::shared_ptr<IWindow> Create(const WindowConfig &window_config = WindowConfig());
    };
}
