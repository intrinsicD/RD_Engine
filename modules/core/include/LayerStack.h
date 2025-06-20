// RDE_Project/modules/core/include/LayerStack.h

#pragma once

#include "Layer.h"

#include <vector>
#include <memory>

namespace RDE {
    class LayerStack {
    public:
        LayerStack() = default;

        ~LayerStack();

        Layer * push_layer(std::shared_ptr<Layer> layer);

        Layer * push_overlay(std::shared_ptr<Layer> overlay);

        void pop_layer(Layer *layer);

        void pop_overlay(Layer *overlay);

        std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_layers.begin(); }

        std::vector<std::shared_ptr<Layer>>::iterator end() { return m_layers.end(); }

        std::vector<std::shared_ptr<Layer>>::reverse_iterator rbegin() { return m_layers.rbegin(); }

        std::vector<std::shared_ptr<Layer>>::reverse_iterator rend() { return m_layers.rend(); }

    private:
        std::vector<std::shared_ptr<Layer>> m_layers;
        unsigned int m_layer_insert_index = 0;
    };
}
