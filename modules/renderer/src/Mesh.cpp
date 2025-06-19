#include "Mesh.h"

namespace RDE {
    Mesh::Mesh(const std::vector<Vertex3D> &vertices, const std::vector<uint32_t> &indices) {
        m_vertex_array = VertexArray::Create();

        auto vertex_buffer = VertexBuffer::Create((float *) vertices.data(), vertices.size() * sizeof(Vertex3D));
        vertex_buffer->set_layout({
                                          {ShaderDataType::Float3, "a_Position"},
                                          {ShaderDataType::Float3, "a_Normal"},
                                          {ShaderDataType::Float2, "a_TexCoord"}
                                  });
        m_vertex_array->add_vertex_buffer(vertex_buffer);

        auto index_buffer = IndexBuffer::Create(indices.data(), indices.size());
        m_vertex_array->set_index_buffer(index_buffer);
    }

    // Static factory function to create a simple cube
    std::shared_ptr<Mesh> Mesh::CreateCube(float size) {
        float s = size * 0.5f;
        std::vector<Vertex3D> vertices = {
// Position           // Normal              // TexCoords
                // Back face (-Z)
                { { -s, -s, -s },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
                { {  s, -s, -s },  { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
                { {  s,  s, -s },  { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
                { { -s,  s, -s },  { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },

                // Front face (+Z)
                { { -s, -s,  s },  { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
                { {  s, -s,  s },  { 0.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
                { {  s,  s,  s },  { 0.0f, 0.0f,  1.0f }, { 1.0f, 1.0f } },
                { { -s,  s,  s },  { 0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f } },

                // Left face (-X)
                { { -s,  s,  s },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
                { { -s,  s, -s },  { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
                { { -s, -s, -s },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
                { { -s, -s,  s },  { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

                // Right face (+X)
                { {  s,  s,  s },  {  1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
                { {  s,  s, -s },  {  1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
                { {  s, -s, -s },  {  1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
                { {  s, -s,  s },  {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

                // Bottom face (-Y)
                { { -s, -s, -s },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
                { {  s, -s, -s },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
                { {  s, -s,  s },  { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
                { { -s, -s,  s },  { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },

                // Top face (+Y)
                { { -s,  s, -s },  { 0.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } },
                { {  s,  s, -s },  { 0.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
                { {  s,  s,  s },  { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
                { { -s,  s,  s },  { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } }
        };

        std::vector<uint32_t> indices = {
                // Back
                0, 1, 2, 2, 3, 0,
                // Front
                4, 5, 6, 6, 7, 4,
                // Left
                8, 9, 10, 10, 11, 8,
                // Right
                12, 13, 14, 14, 15, 12,
                // Bottom
                16, 17, 18, 18, 19, 16,
                // Top
                20, 21, 22, 22, 23, 20
        };

        return std::make_shared<Mesh>(vertices, indices);
    }
}