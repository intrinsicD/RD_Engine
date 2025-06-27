// RDE_Project/modules/core/include/ImGuiLayer.h
#pragma once

#include "ILayer.h"

namespace RDE {
    class ImGuiLayer : public ILayer {
    public:
        ImGuiLayer();

        ~ImGuiLayer() override = default;

        void on_attach(const ApplicationContext &, const FrameContext &) override;

        void on_detach(const ApplicationContext &, const FrameContext &) override;

        void on_event(Event &, const ApplicationContext &, const FrameContext &) override;

        static void begin(const ApplicationContext &, const FrameContext &);

        static void end(const ApplicationContext &, const FrameContext &);

    private:
        float m_time = 0.0f;
    };
}