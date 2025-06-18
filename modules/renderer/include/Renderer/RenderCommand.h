// RDE_Project/modules/renderer/include/Renderer/RenderCommand.h
#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/GraphicsAPI.h"

class RenderCommand {
public:
    // This is not a class to be instantiated.
    RenderCommand() = delete;

    static void Init() { s_graphics_api->Init(); }

    static void SetClearColor(float r, float g, float b, float a) {
        s_graphics_api->SetClearColor(r, g, b, a);
    }

    static void Clear() { s_graphics_api->Clear(); }

    static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) {
        s_graphics_api->DrawIndexed(vertexArray, indexCount);
    }

private:
    static std::unique_ptr<GraphicsAPI> s_graphics_api;
};