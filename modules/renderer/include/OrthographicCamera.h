// RDE_Project/modules/renderer/include/Renderer/OrthographicCamera.h
#pragma once

#include "Camera.h"

namespace RDE {
    class OrthographicCamera : public Camera {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);

        void set_projection(float left, float right, float bottom, float top);

    private:
        void recalculate_projection();

    private:
        float m_left;
        float m_right;
        float m_bottom;
        float m_top;
    };
}