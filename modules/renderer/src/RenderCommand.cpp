//
// Created by alex on 18.06.25.
//

#include "Renderer/RenderCommand.h"
#include "Renderer/GraphicsAPI.h"
namespace RDE {
    std::unique_ptr<GraphicsAPI> RenderCommand::s_graphics_api = GraphicsAPI::Create();
}