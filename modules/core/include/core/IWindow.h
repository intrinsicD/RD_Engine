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

    class IWindow {
    public:
        using EventCallbackFn = std::function<void(Event &)>;

        virtual ~IWindow() = default;

        virtual void set_event_callback(const EventCallbackFn &callback) = 0;

        virtual void poll_events() = 0;

        virtual bool should_close() = 0; // Better name than 'on_update' for the loop condition

        [[nodiscard]] virtual int get_width() const = 0;

        [[nodiscard]] virtual int get_height() const = 0;

        [[nodiscard]] virtual void *get_native_handle() const = 0;

        static std::unique_ptr<IWindow> Create(const WindowConfig &config = WindowConfig());
    };
}
