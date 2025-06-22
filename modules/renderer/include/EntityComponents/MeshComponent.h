#pragma once

#include "AssetHandle.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>

namespace RDE {
    struct MeshComponent {
        AssetHandle mesh_handle;
        std::shared_ptr<Texture2D> texture = nullptr;
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    };
}