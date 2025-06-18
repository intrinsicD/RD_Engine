// RDE_Project/modules/core/include/Core/Layer.h

#pragma once

#include "Core/Base.h"
#include "Core/Events/Event.h"

class Layer {
public:
    Layer(const std::string &name = "Layer");

    virtual ~Layer() = default;

    // Called when the layer is pushed onto the layer stack.
    virtual void OnAttach() {
    }

    // Called when the layer is popped from the layer stack.
    virtual void OnDetach() {
    }

    // Called every frame during the main application loop.
    virtual void OnUpdate() {
    }

    // Called when an event is sent to the layer.
    virtual void OnEvent(Event &event) {
    }

    const std::string &get_name() const { return m_debug_name; }

protected:
    std::string m_debug_name;
};
