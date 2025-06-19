#include "Mesh.h"

namespace RDE {
    Mesh::Mesh(const std::vector<Vertex3D> &vertices, const std::vector<uint32_t> &indices) {
        m_vertex_array = VertexArray::Create();

        auto vertex_buffer = VertexBuffer::Create((float *) vertices.data(), vertices.size() * sizeof(Vertex3D));
        vertex_buffer->set_layout({
                                          {ShaderDataType::Float3, "a_Position"}
                                  });
        m_vertex_array->add_vertex_buffer(vertex_buffer);

        auto index_buffer = IndexBuffer::Create(indices.data(), indices.size());
        m_vertex_array->set_index_buffer(index_buffer);
    }

    // Static factory function to create a simple cube
    std::shared_ptr<Mesh> Mesh::CreateCube(float size) {
        float half_size = size * 0.5f;
        std::vector<Vertex3D> vertices = {
                {{-half_size, -half_size, half_size}},
                {{half_size,  -half_size, half_size}},
                {{half_size,  half_size,  half_size}},
                {{-half_size, half_size,  half_size}},
                {{-half_size, -half_size, -half_size}},
                {{half_size,  -half_size, -half_size}},
                {{half_size,  half_size,  -half_size}},
                {{-half_size, half_size,  -half_size}}
        };

        std::vector<uint32_t> indices = {
                0, 1, 2, 2, 3, 0, // Front face
                1, 5, 6, 6, 2, 1, // Right face
                5, 4, 7, 7, 6, 5, // Back face
                4, 0, 3, 3, 7, 4, // Left face
                3, 2, 6, 6, 7, 3, // Top face
                4, 5, 1, 1, 0, 4  // Bottom face
        };

        return std::make_shared<Mesh>(vertices, indices);
    }
}