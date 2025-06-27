// RDE_Project/modules/core/include/ILayerStack.h

#pragma once

#include "ILayer.h"
#include "ApplicationContext.h"
#include "FrameContext.h"

#include <vector>
#include <memory>
#include <algorithm>

namespace RDE {
    class LayerStack {
    public:
        LayerStack(ApplicationContext &app_context, FrameContext &frame_context) : app_context(app_context), frame_context(frame_context) {
            // Initialize the layer stack with a default layer if needed.
            // This could be a base layer that handles common functionality.
            // m_layers.emplace_back(std::make_shared<DefaultLayer>());
        }

        ~LayerStack() {
            // Clean up all layers
            for (auto &layer: m_layers) {
                layer->on_detach(app_context, frame_context);
            }
        }

        ILayer *push_layer(std::shared_ptr<ILayer> layer) {
            auto it = m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer));
            m_layer_insert_index++;
            (*it)->on_attach(app_context, frame_context); // Call the attach hook.
            return it->get();
        }

        ILayer *push_overlay(std::shared_ptr<ILayer> overlay) {
            // Overlays are always added to the very end of the list.
            m_layers.emplace_back(std::move(overlay));
            m_layers.back()->on_attach(app_context, frame_context); // Call the attach hook.
            return m_layers.back().get();
        }

        void pop_layer(ILayer *layer) {
            auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layer_insert_index,
                                   [layer](const std::shared_ptr<ILayer>& l) { return l.get() == layer; });

            if (it != m_layers.begin() + m_layer_insert_index) {
                (*it)->on_detach(app_context, frame_context);
                m_layers.erase(it);
                m_layer_insert_index--; // Decrement the boundary index as a normal layer was removed.
            }
        }

        void pop_overlay(ILayer *overlay) {
            auto it = std::find_if(m_layers.begin() + m_layer_insert_index, m_layers.end(),
                       [overlay](const std::shared_ptr<ILayer>& l) { return l.get() == overlay; });

            if (it != m_layers.end()) {
                (*it)->on_detach(app_context, frame_context);
                m_layers.erase(it); // Simply remove the overlay. The insert index is not affected.
            }
        }

        std::vector<std::shared_ptr<ILayer> >::iterator begin() { return m_layers.begin(); }

        std::vector<std::shared_ptr<ILayer> >::iterator end() { return m_layers.end(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rbegin() { return m_layers.rbegin(); }

        std::vector<std::shared_ptr<ILayer> >::reverse_iterator rend() { return m_layers.rend(); }

    private:
        ApplicationContext &app_context;
        FrameContext &frame_context;

        std::vector<std::shared_ptr<ILayer> > m_layers;
        unsigned int m_layer_insert_index = 0;
    };
}
