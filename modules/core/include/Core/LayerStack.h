// RDE_Project/modules/core/include/Core/LayerStack.h

#pragma once

#include "Core/Base.h"
#include "Core/Layer.h"

#include <vector>

class LayerStack {
public:
    LayerStack() = default;

    ~LayerStack();

    void PushLayer(Layer *layer);

    void PushOverlay(Layer *overlay);

    void PopLayer(Layer *layer);

    void PopOverlay(Layer *overlay);

    std::vector<Layer *>::iterator begin() { return m_layers.begin(); }
    std::vector<Layer *>::iterator end() { return m_layers.end(); }
    std::vector<Layer *>::reverse_iterator rbegin() { return m_layers.rbegin(); }
    std::vector<Layer *>::reverse_iterator rend() { return m_layers.rend(); }

private:
    std::vector<Layer *> m_layers;
    unsigned int m_layer_insert_index = 0;
};
