// RDE_Project/modules/platform/opengl/OpenGLGraphicsAPI.h
#pragma once

#include "Renderer/GraphicsAPI.h"
namespace RDE {
    class OpenGLGraphicsAPI : public GraphicsAPI {
    public:
        virtual void init() override;

        virtual void set_clear_color(float r, float g, float b, float a) override;

        virtual void clear() override;

        virtual void draw_indexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) override;
    };
}