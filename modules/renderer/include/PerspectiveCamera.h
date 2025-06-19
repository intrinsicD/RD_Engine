#pragma once

#include "Camera.h"

namespace RDE {
    class PerspectiveCamera : public Camera {
    public:
        PerspectiveCamera(float vertical_fov, float aspect_ratio, float near_clip, float far_clip);

        void set_projection(float vertical_fov, float aspect_ratio, float near_clip, float far_clip);

        void set_aspect_ratio(float aspect_ratio);

    private:
        void recalculate_projection();

    private:
        float m_vertical_fov;
        float m_aspect_ratio;
        float m_near_clip;
        float m_far_clip;
    };
}