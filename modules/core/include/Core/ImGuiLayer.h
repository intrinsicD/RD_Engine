// RDE_Project/modules/core/include/Core/ImGuiLayer.h
#pragma once
#include "Core/Layer.h"

namespace RDE {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();

        ~ImGuiLayer();

        virtual void on_attach() override;

        virtual void on_detach() override;

        virtual void on_event(Event &e) override;

        void begin();

        void end();

    private:
        float m_time = 0.0f;
    };
}