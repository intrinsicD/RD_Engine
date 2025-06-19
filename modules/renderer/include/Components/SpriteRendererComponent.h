// RDE_Project/modules/renderer/include/Components/SpriteComponent.h
#pragma once

#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>

namespace RDE {
    struct SpriteRendererComponent {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture2D> texture;
        float tiling_factor = 1.0f;

        SpriteRendererComponent() = default;

        SpriteRendererComponent(const SpriteRendererComponent &) = default;

        SpriteRendererComponent(const glm::vec4 &color) : color(color) {}
    };
}