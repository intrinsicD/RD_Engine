// RDE_Project/modules/renderer/include/Renderer/GraphicsAPI.h
#pragma once

#include "Renderer/VertexArray.h"
#include <memory>

// NOTE-DESIGN: In a real engine, you would pass math types like vectors/matrices here.
// For now, we will use floats for color.

class GraphicsAPI {
public:
    enum class API {
        None = 0, OpenGL = 1
    };

public:
    virtual ~GraphicsAPI() = default;

    virtual void Init() = 0;

    virtual void SetClearColor(float r, float g, float b, float a) = 0;

    virtual void Clear() = 0;

    virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

    static API GetAPI() { return s_api; }

    static std::unique_ptr<GraphicsAPI> Create();

private:
    static API s_api;
};