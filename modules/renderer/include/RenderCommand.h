// RDE_Project/modules/renderer/include/Renderer/RenderCommand.h
#pragma once

#include "VertexArray.h"
#include "GraphicsAPI.h"
namespace RDE {
    class RenderCommand {
    public:
        // This is not a class to be instantiated.
        RenderCommand() = delete;

        static void Init() { s_graphics_api->init(); }

        static void SetClearColor(float r, float g, float b, float a) {
            s_graphics_api->set_clear_color(r, g, b, a);
        }

        static void Clear() { s_graphics_api->clear(); }

        static void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) {
            s_graphics_api->draw_indexed(vertexArray, indexCount);
        }

        static void SetDepthTest(bool enabled) {
            s_graphics_api->set_depth_test(enabled);
        }

        static void SetBlending(bool enabled) {
            s_graphics_api->set_blending(enabled);
        }

    private:
        static std::unique_ptr<GraphicsAPI> s_graphics_api;
    };
}