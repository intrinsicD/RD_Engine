#pragma once

#include "Buffer.h"
#include "VertexArray.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace RDE {
    struct Vertex3D {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coords;
    };

    class Mesh {
    public:
        Mesh(const std::vector<Vertex3D> &vertices, const std::vector<uint32_t> &indices);

        const std::shared_ptr<VertexArray> &get_vertex_array() const { return m_vertex_array; }

        static std::shared_ptr<Mesh> CreateCube(float size = 1.0f);

    private:
        std::shared_ptr<VertexArray> m_vertex_array;
        // We don't need to store the raw vertex/index data after uploading to GPU
    };
}