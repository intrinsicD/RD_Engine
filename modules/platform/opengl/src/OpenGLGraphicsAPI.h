// RDE_Project/modules/platform/opengl/OpenGLGraphicsAPI.h
#pragma once

#include "GraphicsAPI.h"

namespace RDE {
    class OpenGLGraphicsAPI : public GraphicsAPI {
    public:
        void init() override;

        void set_clear_color(float r, float g, float b, float a) override;

        void set_depth_test(bool enabled) override;

        void set_blending(bool enabled) override;

        void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        void clear() override;

        void draw_indexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) override;
    };
}