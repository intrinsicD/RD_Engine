//ral/CommandBufferTypes.h
#pragma once

#include "Common.h"

#include <vector>

namespace RAL {
    enum class LoadOp {
        Load,   // Preserve the previous contents of the attachment
        Clear,  // Clear the attachment to a specific value
        DontCare // The driver is free to discard the contents
    };

    enum class StoreOp {
        Store,    // Store the results of the render pass
        DontCare  // The driver is free to discard the results
    };

    struct RenderPassAttachment {
        TextureHandle texture; // The texture to render into
        LoadOp loadOp = LoadOp::DontCare;
        StoreOp storeOp = StoreOp::Store;
        // The color to clear to if loadOp is Clear
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        // Depth/stencil clear values
        float clearDepth = 1.0f;
        uint32_t clearStencil = 0;
    };

    struct RenderPassDescription {
        std::vector<RenderPassAttachment> colorAttachments;
        // Optional depth attachment
        RenderPassAttachment depthStencilAttachment;
    };
}