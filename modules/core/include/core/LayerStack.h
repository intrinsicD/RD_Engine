#pragma once

#include "ILayer.h"

#include <memory>
#include <vector>
#include <algorithm>

namespace RDE {
    class LayerStack {
    public:
        LayerStack() = default;

        ~LayerStack() = default;

        ILayer *push_layer(std::shared_ptr<ILayer> layer) {
            auto it = m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer));
            m_layer_insert_index++;
            (*it)->on_attach(); // Call the attach hook.
            return it->get();
        }

        ILayer *push_overlay(std::shared_ptr<ILayer> overlay) {
            // Overlays are always added to the very end of the list.
            m_layers.emplace_back(std::move(overlay));
            m_layers.back()->on_attach(); // Call the attach hook.
            return m_layers.back().get();
        }

        void pop_layer(ILayer *layer) {
            auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layer_insert_index,
                                   [layer](const std::shared_ptr<ILayer> &l) { return l.get() == layer; });

            if (it != m_layers.begin() + m_layer_insert_index) {
                (*it)->on_detach();
                m_layers.erase(it);
                m_layer_insert_index--; // Decrement the boundary index as a normal layer was removed.
            }
        }

        void pop_overlay(ILayer *overlay) {
            auto it = std::find_if(m_layers.begin() + m_layer_insert_index, m_layers.end(),
                                   [overlay](const std::shared_ptr<ILayer> &l) { return l.get() == overlay; });

            if (it != m_layers.end()) {
                (*it)->on_detach();
                m_layers.erase(it); // Simply remove the overlay. The insert index is not affected.
            }
        }

        std::vector<std::shared_ptr<ILayer> >::iterator begin() { return m_layers.begin(); }

        std::vector<std::shared_ptr<ILayer> >::iterator end() { return m_layers.end(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rbegin() { return m_layers.rbegin(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rend() { return m_layers.rend(); }

    private:
        std::vector<std::shared_ptr<ILayer> > m_layers;
        unsigned int m_layer_insert_index = 0;
    };
}