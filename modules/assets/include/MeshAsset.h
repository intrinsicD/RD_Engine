// file: modules/assets/include/Assets/MeshAsset.h
#pragma once

#include "IAsset.h"

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace RDE {
    struct Vertex {
        Vertex() = default;

        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        glm::vec2 tex_coords{0.0f};
        // glm::vec3 tangent; // For future normal mapping

        bool operator==(const Vertex &other) const {
            return position == other.position &&
                   normal == other.normal &&
                   tex_coords == other.tex_coords;
        }
    };

    struct MeshAsset : public IAsset {
        // --- DATA POPULATED BY ASSETMANAGER ---
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // --- DATA POPULATED BY RENDERER ---
        // The IDs of the GPU buffers for this mesh.
        uint32_t vao_id = 0; // Vertex Array Object
        uint32_t vbo_id = 0; // Vertex Buffer Object
        uint32_t ebo_id = 0; // Element Buffer Object
    };
} // namespace RDE
