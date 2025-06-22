// file: modules/assets/include/Assets/ShaderAsset.h
#pragma once

#include "IAsset.h"

#include <string>
#include <cstdint>

namespace RDE {
    struct ShaderAsset : public IAsset {
        // --- DATA POPULATED BY ASSETMANAGER ---
        std::string vertex_source;
        std::string fragment_source;
        std::string geometry_source;
        std::string compute_source;

        // --- DATA POPULATED BY RENDERER ---
        // The ID of the linked shader program on the GPU.
        uint32_t renderer_id = 0;
    };

} // namespace RDE