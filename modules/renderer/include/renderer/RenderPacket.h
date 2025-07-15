#pragma once

#include "ral/Common.h"

#include <vector>
#include <glm/mat4x4.hpp>

namespace RDE {
    struct RenderGpuGeometry;
    struct RenderGpuMaterial; // Let's rename RenderGpuPipeline to this for clarity
}

namespace RDE {
    struct RenderPacket {
        const RenderGpuMaterial* material;
        const RenderGpuGeometry* geometry;

        glm::mat4 model_matrix;
    };

    using View = std::vector<RenderPacket>;
}