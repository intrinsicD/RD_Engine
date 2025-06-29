#pragma once

#include "Common.h"
#include <vector>

namespace RAL {
    struct ClearColorValue {
        float r, g, b, a;
    };

    struct ClearDepthStencilValue {
        float depth;
        uint32_t stencil;
    };

    using ClearValue = std::variant<ClearColorValue, ClearDepthStencilValue>;

    // Describes how to begin a render pass instance.
    struct RenderPassBeginInfo {
        std::vector<TextureHandle> color_attachments;
        std::vector<ClearValue> color_clear_values;
        TextureHandle depth_stencil_attachment;
        ClearValue depth_stencil_clear_value;
        // ... other info like render area ...
    };
}
