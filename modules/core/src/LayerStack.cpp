// RDE_Project/modules/core/src/LayerStack.cpp

#include "LayerStack.h"

#include <algorithm>

namespace RDE {
    LayerStack::~LayerStack() {
        for (const auto &layer: m_layers) {
            layer->on_detach();
        }
        m_layers.clear();
    }

    Layer *LayerStack::push_layer(std::unique_ptr<Layer> layer) {
        auto *raw_ptr = m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer))->get();
        m_layer_insert_index++;
        raw_ptr->on_attach();
        // Call on_attach for the new layer via raw pointer because it is now owned by the stack and layer is null
        return raw_ptr;
    }

    Layer *LayerStack::push_overlay(std::unique_ptr<Layer> overlay) {
        auto *raw_ptr = m_layers.emplace_back(std::move(overlay)).get();
        raw_ptr->on_attach();
        return raw_ptr;
    }

    void LayerStack::pop_layer(Layer *layer) {
        auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layer_insert_index,
                               [&](const std::unique_ptr<Layer> &ptr) {
                                   return ptr.get() == layer;
                               });

        if (it != m_layers.begin() + m_layer_insert_index) {
            layer->on_detach();
            m_layers.erase(it);
            m_layer_insert_index--;
        }
    }

    void LayerStack::pop_overlay(Layer *overlay) {
        auto it = std::find_if(m_layers.begin() + m_layer_insert_index, m_layers.end(),
                               [&](const std::unique_ptr<Layer> &ptr) {
                                   return ptr.get() == overlay;
                               });

        if (it != m_layers.end()) {
            overlay->on_detach();
            m_layers.erase(it);
        }
    }
}
