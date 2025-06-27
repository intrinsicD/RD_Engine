#pragma once

#include <string>
#include <utility>

namespace RDE {
    struct ApplicationContext;
    struct FrameContext;

    class Event;

    /*
     * The Layer's Job: To manipulate the state of the Scene.
     * It creates entities, moves them, changes their components, and responds to game events.
     * It describes what the world should look like.
     */

    class ILayer {
    public:
        explicit ILayer(std::string name = "Layer") : m_debug_name(std::move(name)) {
        }

        virtual ~ILayer() = default;

        virtual void on_attach(const ApplicationContext &, const FrameContext &) {
        }

        virtual void on_detach(const ApplicationContext &, const FrameContext &) {
        }

        virtual void on_variable_update(const ApplicationContext &, const FrameContext &) {
        }

        virtual void on_fixed_update(const ApplicationContext &, const FrameContext &) {
        }

        virtual void on_gui_render(const ApplicationContext &context, const FrameContext &frame_context) {
        }

        virtual void on_event(Event &event, const ApplicationContext &context, const FrameContext &frame_context) {
        }

        [[nodiscard]] const std::string &get_name() const { return m_debug_name; }

    protected:
        std::string m_debug_name;
    };
}
