#pragma once

namespace RAL{
    class CommandBuffer;
}
namespace RDE {
    class Event; // Forward declaration of Event class
    class ILayer {
    public:
        virtual ~ILayer() = default;

        virtual void on_attach() = 0;

        virtual void on_detach() = 0;

        virtual void on_update(float delta_time) = 0;

        virtual void on_render(RAL::CommandBuffer *cmd) = 0;

        virtual void on_render_gui() = 0;

        virtual void on_event(Event &e) = 0;

        virtual const char * get_name() const = 0;
    };
}