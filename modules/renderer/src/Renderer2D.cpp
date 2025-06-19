#include "Renderer2D.h"
#include "RenderCommand.h"
#include "VertexArray.h"
#include "Shader.h"
#include "FileIO.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RDE {
    struct QuadVertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 tex_coord;
        float texture_index;
        float tiling_factor;
    };

    struct Renderer2DData {
        static const uint32_t max_quads = 10000;
        static const uint32_t max_vertices = max_quads * 4;
        static const uint32_t max_indices = max_quads * 6;
        static const uint32_t max_texture_slots = 32; // TODO: Query GPU for this limit

        std::shared_ptr<VertexArray> quad_vertex_array;
        std::shared_ptr<VertexBuffer> quad_vertex_buffer;
        std::shared_ptr<Shader> texture_shader;
        std::shared_ptr<Texture2D> white_texture;

        uint32_t quad_index_count = 0;
        QuadVertex *quad_vertex_buffer_base = nullptr;
        QuadVertex *quad_vertex_buffer_ptr = nullptr;

        std::array<std::shared_ptr<Texture2D>, max_texture_slots> texture_slots;
        uint32_t texture_slot_index = 1; // 0 = white texture

        glm::vec4 quad_vertex_positions[4];

        Renderer2D::Statistics stats;
    };

    static Renderer2DData s_data;

    void Renderer2D::Init() {
        s_data.quad_vertex_array = VertexArray::Create();

        s_data.quad_vertex_buffer = VertexBuffer::Create(s_data.max_vertices * sizeof(QuadVertex));
        s_data.quad_vertex_buffer->set_layout({
                                                      {ShaderDataType::Float3, "a_Position"},
                                                      {ShaderDataType::Float4, "a_Color"},
                                                      {ShaderDataType::Float2, "a_TexCoord"},
                                                      {ShaderDataType::Float,  "a_TexIndex"},
                                                      {ShaderDataType::Float,  "a_TilingFactor"}
                                              });
        s_data.quad_vertex_array->add_vertex_buffer(s_data.quad_vertex_buffer);

        s_data.quad_vertex_buffer_base = new QuadVertex[s_data.max_vertices];

        uint32_t *quad_indices = new uint32_t[s_data.max_indices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_data.max_indices; i += 6) {
            quad_indices[i + 0] = offset + 0;
            quad_indices[i + 1] = offset + 1;
            quad_indices[i + 2] = offset + 2;
            quad_indices[i + 3] = offset + 2;
            quad_indices[i + 4] = offset + 3;
            quad_indices[i + 5] = offset + 0;
            offset += 4;
        }
        auto quad_ib = IndexBuffer::Create(quad_indices, s_data.max_indices);
        s_data.quad_vertex_array->set_index_buffer(quad_ib);
        delete[] quad_indices;

        // Create a 1x1 white texture for rendering solid colors
        s_data.white_texture = Texture2D::Create(1, 1);
        uint32_t white_texture_data = 0xffffffff;
        s_data.white_texture->set_data(&white_texture_data, sizeof(uint32_t));

        // Create and configure the shader
        s_data.texture_shader = Shader::CreateFromFile(FileIO::GetPath("assets/shaders/Texture.vert"),
                                                       FileIO::GetPath("assets/shaders/Texture.frag"));
        s_data.texture_shader->bind();
        int32_t samplers[s_data.max_texture_slots];
        for (int32_t i = 0; i < s_data.max_texture_slots; i++)
            samplers[i] = i;
        s_data.texture_shader->set_int_array("u_Textures", samplers, s_data.max_texture_slots);

        s_data.texture_slots[0] = s_data.white_texture;

        // Define standard vertex positions for a quad
        s_data.quad_vertex_positions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    void Renderer2D::Shutdown() {
        delete[] s_data.quad_vertex_buffer_base;
    }

    void Renderer2D::BeginPass() {
        // Set the graphics state required for 2D rendering.
        // This is crucial for preventing conflicts with the 3D renderer.
        RenderCommand::SetDepthTest(false);
        RenderCommand::SetBlending(true);

        // Use a static orthographic camera for all screen-space rendering.
        // We assume a window size, this should be updated on viewport resize.
        glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
        s_data.texture_shader->bind();
        s_data.texture_shader->set_mat4("u_ViewProjection", projection);

        StartBatch();
    }

    void Renderer2D::EndPass() {
        // Flush any remaining data in the current batch.
        uint32_t data_size = (uint8_t *) s_data.quad_vertex_buffer_ptr - (uint8_t *) s_data.quad_vertex_buffer_base;
        if (data_size > 0) {
            s_data.quad_vertex_buffer->set_data(s_data.quad_vertex_buffer_base, data_size);

            for (uint32_t i = 0; i < s_data.texture_slot_index; i++)
                s_data.texture_slots[i]->bind(i);

            RenderCommand::DrawIndexed(s_data.quad_vertex_array, s_data.quad_index_count);
            s_data.stats.draw_calls++;
        }
    }

    void Renderer2D::StartBatch() {
        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;
        s_data.texture_slot_index = 1;
    }

    void Renderer2D::NextBatch() {
        EndPass();
        StartBatch();
    }

    Renderer2D::Statistics Renderer2D::GetStats() {
        return s_data.stats;
    }

    void Renderer2D::ResetStats() {
        memset(&s_data.stats, 0, sizeof(Statistics));
    }

    void Renderer2D::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color) {
        DrawQuad(transform, nullptr, 1.0f, color);
    }

    void
    Renderer2D::DrawQuad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                         const glm::vec4 &tint_color) {
        if (s_data.quad_index_count >= Renderer2DData::max_indices)
            NextBatch();

        float texture_index = 0.0f; // Default to white texture
        if (texture) {
            // Find if texture is already in the batch
            for (uint32_t i = 1; i < s_data.texture_slot_index; i++) {
                if (*s_data.texture_slots[i].get() == *texture.get()) {
                    texture_index = (float) i;
                    break;
                }
            }

            // If not found, add it to the batch
            if (texture_index == 0.0f) {
                if (s_data.texture_slot_index >= Renderer2DData::max_texture_slots)
                    NextBatch();

                texture_index = (float) s_data.texture_slot_index;
                s_data.texture_slots[s_data.texture_slot_index] = texture;
                s_data.texture_slot_index++;
            }
        }

        const glm::vec2 tex_coords[] = {{0.0f, 0.0f},
                                        {1.0f, 0.0f},
                                        {1.0f, 1.0f},
                                        {0.0f, 1.0f}};

        for (size_t i = 0; i < 4; i++) {
            s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coord = tex_coords[i];
            s_data.quad_vertex_buffer_ptr->texture_index = texture_index;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        s_data.quad_index_count += 6;
        s_data.stats.quad_count++;
    }

    // --- Implement all the convenience overloads by delegating to the base DrawQuad(mat4, ...) ---

    void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {
        glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, color);
    }

    void
    Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                         float tiling_factor, const glm::vec4 &tint_color) {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tiling_factor, tint_color);
    }

    void
    Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                         float tiling_factor, const glm::vec4 &tint_color) {
        glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, texture, tiling_factor, tint_color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                     const glm::vec4 &color) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation_radians, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation_radians,
                                     const glm::vec4 &color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), rotation_radians, {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                     const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                                     const glm::vec4 &tint_color) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation_radians, texture, tiling_factor, tint_color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation_radians,
                                     const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                                     const glm::vec4 &tint_color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), rotation_radians, {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, texture, tiling_factor, tint_color);
    }
}