#pragma once

#include "core/events/Event.h"

struct GLFWwindow;

namespace RDE {
    class Application {
    public:
        virtual ~Application() = default;

        virtual void run() = 0;

    private:
        virtual bool init() = 0;

        virtual void shutdown() = 0;

        virtual void on_update(float delta_time) = 0;

        virtual void on_render() = 0;

        virtual void on_event(Event &e) = 0;
    };
}
