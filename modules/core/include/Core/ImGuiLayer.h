// RDE_Project/modules/core/include/Core/ImGuiLayer.h
#pragma once
#include "Core/Layer.h"

class ImGuiLayer : public Layer {
public:
    ImGuiLayer();

    ~ImGuiLayer();

    virtual void OnAttach() override;

    virtual void OnDetach() override;

    virtual void OnEvent(Event &e) override;

    void Begin();

    void End();

private:
    float m_time = 0.0f;
};
