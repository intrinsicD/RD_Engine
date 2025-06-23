// RDE_Project/modules/core/include/ImGuiLayer.h
#pragma once

#include "ILayer.h"

namespace RDE {
    class ImGuiLayer : public ILayer {
    public:
        ImGuiLayer();

        ~ImGuiLayer();

        void on_attach() override;

        void on_detach() override;

        void on_event(Event &e) override;

        void begin();

        void end();

    private:
        float m_time = 0.0f;
    };
}