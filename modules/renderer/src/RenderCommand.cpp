//
// Created by alex on 18.06.25.
//

#include "../include/RenderCommand.h"
#include "../include/GraphicsAPI.h"
namespace RDE {
    std::unique_ptr<GraphicsAPI> RenderCommand::s_graphics_api = GraphicsAPI::Create();
}