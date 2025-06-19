#pragma once

#include "Mesh.h"

#include <glm/glm.hpp>
#include <memory>

namespace RDE {
    struct MeshComponent {
        std::shared_ptr<Mesh> mesh = nullptr;
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

        MeshComponent() = default;

        MeshComponent(const MeshComponent &) = default;

        MeshComponent(const std::shared_ptr<Mesh> &mesh, const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f})
                : mesh(mesh), color(color) {}
    };
}