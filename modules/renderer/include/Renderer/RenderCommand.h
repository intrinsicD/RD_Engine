// RDE_Project/modules/renderer/include/Renderer/RenderCommand.h
#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/GraphicsAPI.h"
namespace RDE {
    class RenderCommand {
    public:
        // This is not a class to be instantiated.
        RenderCommand() = delete;

        static void init() { s_graphics_api->init(); }

        static void set_clear_color(float r, float g, float b, float a) {
            s_graphics_api->set_clear_color(r, g, b, a);
        }

        static void clear() { s_graphics_api->clear(); }

        static void draw_indexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) {
            s_graphics_api->draw_indexed(vertexArray, indexCount);
        }

    private:
        static std::unique_ptr<GraphicsAPI> s_graphics_api;
    };
}