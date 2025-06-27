#pragma once

#include "graph/RenderGraphResource.h"

namespace RDE{
    class RenderGraph;
    class ImDrawData;

    void setup_forward_opaque_pass(RenderGraph& rg, RGResourceHandle final_render_target);
}