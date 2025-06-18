// RDE_Project/modules/platform/opengl/OpenGLGraphicsAPI.h
#pragma once

#include "Renderer/GraphicsAPI.h"
namespace RDE {
    class OpenGLGraphicsAPI : public GraphicsAPI {
    public:
        virtual void Init() override;

        virtual void SetClearColor(float r, float g, float b, float a) override;

        virtual void Clear() override;

        virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) override;
    };
}