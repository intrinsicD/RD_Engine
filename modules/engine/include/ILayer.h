#pragma once

#include "events/Event.h"

#include <string>

namespace RDE {
    class ILayer {
    public:
        ILayer(const std::string &name = "Layer") : m_debug_name(name) {
        }

        virtual ~ILayer() = default;

        // Called when the layer is pushed onto the layer stack.
        virtual void on_attach() {
        }

        // Called when the layer is popped from the layer stack.
        virtual void on_detach() {
        }

        virtual void on_begin_frame() {
        }

        virtual void on_event(Event &event) {
        }

        virtual void on_fixed_update(float fixed_timestep) {
        }

        // Called every frame during the main application loop.
        virtual void on_variable_update(float delta_time) {
        }

        virtual void on_render_submission() {
        }

        // Called every frame to render the GUI.
        virtual void on_gui_render() {
        }

        const std::string &get_name() const { return m_debug_name; }

    protected:
        std::string m_debug_name;
    };
}
