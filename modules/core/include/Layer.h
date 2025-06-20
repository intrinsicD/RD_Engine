// RDE_Project/modules/core/include/Layer.h

#pragma once

#include "Base.h"
#include "Events/Event.h"

namespace RDE {
    class Layer {
    public:
        Layer(const std::string &name = "Layer");

        virtual ~Layer() = default;

        // Called when the layer is pushed onto the layer stack.
        virtual void on_attach() {
        }

        // Called when the layer is popped from the layer stack.
        virtual void on_detach() {
        }

        // Called every frame during the main application loop.
        virtual void on_update(float delta_time) {
        }

        // Called every frame to render the GUI.
        virtual void on_gui_render() {
        }

        // Called when an event is sent to the layer.
        virtual void on_event(Event &event) {
        }

        const std::string &get_name() const { return m_debug_name; }

    protected:
        std::string m_debug_name;
    };
}
