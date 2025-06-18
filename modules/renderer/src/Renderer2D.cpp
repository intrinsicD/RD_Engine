#include "Renderer/Renderer2D.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/RenderCommand.h"
#include "Core/FileIO.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace RDE {
    struct QuadVertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 tex_coord;
        float tex_index;
        float tiling_factor;
    };

    struct Renderer2DData {
        static const uint32_t max_quads = 10000;
        static const uint32_t max_vertices = max_quads * 4;
        static const uint32_t max_indices = max_quads * 6;
        static const uint32_t max_texture_slots = 32; // TODO: RenderCaps

        std::shared_ptr<VertexArray> quad_vertex_array;
        std::shared_ptr<VertexBuffer> qad_vertex_buffer;
        std::shared_ptr<Shader> texture_shader;
        std::shared_ptr<Texture2D> white_texture;

        uint32_t quad_index_count = 0;
        QuadVertex *quad_vertex_buffer_base = nullptr;
        QuadVertex *quad_vertex_buffer_ptr = nullptr;

        std::array<std::shared_ptr<Texture2D>, max_texture_slots> TextureSlots;
        uint32_t texture_slot_index = 1; // 0 = white texture

        glm::vec4 quad_vertex_positions[4];
        Renderer2D::Statistics stats;
    };

    static Renderer2DData s_data;

    void Renderer2D::Init() {
        s_data.quad_vertex_array = VertexArray::Create();
        s_data.qad_vertex_buffer = VertexBuffer::Create(s_data.max_vertices * sizeof(QuadVertex));
        s_data.qad_vertex_buffer->set_layout({
                                                     {ShaderDataType::Float3, "a_Position"},
                                                     {ShaderDataType::Float4, "a_Color"},
                                                     {ShaderDataType::Float2, "a_TexCoord"},
                                                     {ShaderDataType::Float,  "a_TexIndex"},
                                                     {ShaderDataType::Float,  "a_TilingFactor"}
                                             });
        s_data.quad_vertex_array->add_vertex_buffer(s_data.qad_vertex_buffer);
        s_data.quad_vertex_buffer_base = new QuadVertex[s_data.max_vertices];

        uint32_t *quadIndices = new uint32_t[s_data.max_indices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_data.max_indices; i += 6) {
            quadIndices[i + 0] = offset + 0; // Bottom-left
            quadIndices[i + 1] = offset + 1; // Bottom-right
            quadIndices[i + 2] = offset + 2; // Top-right

            quadIndices[i + 3] = offset + 2; // Top-right
            quadIndices[i + 4] = offset + 3; // Top-left
            quadIndices[i + 5] = offset + 0; // Bottom-left

            offset += 4; // Move to the next set of 4 vertices
        }
        auto quadIB = IndexBuffer::Create(quadIndices, s_data.max_indices);
        s_data.quad_vertex_array->set_index_buffer(quadIB);
        delete[] quadIndices;

        s_data.white_texture = Texture2D::Create(1, 1);
        uint32_t white_texture_data = 0xffffffff;
        s_data.white_texture->set_data(&white_texture_data, sizeof(uint32_t)); // Now uncommented and functional

        int32_t samplers[s_data.max_texture_slots];
        for (uint32_t i = 0; i < s_data.max_texture_slots; i++) samplers[i] = i;

        s_data.texture_shader = Shader::CreateFromFile(FileIO::get_path("assets/shaders/Texture.vert"),
                                                       FileIO::get_path("assets/shaders/Texture.frag"));
        s_data.texture_shader->bind();
        s_data.texture_shader->set_int_array("u_Textures", samplers, s_data.max_texture_slots);
        s_data.TextureSlots[0] = s_data.white_texture;

        s_data.quad_vertex_positions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_data.quad_vertex_positions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};


    }

    void Renderer2D::Shutdown() { delete[] s_data.quad_vertex_buffer_base; }

    void Renderer2D::begin_scene(const OrthographicCamera &camera) {
        s_data.texture_shader->bind();
        s_data.texture_shader->set_mat4("u_ViewProjection", camera.GetViewProjectionMatrix());
        reset_stats(); // Reset stats at the beginning of the scene
        s_data.quad_index_count = 0;
        s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;
        s_data.texture_slot_index = 1;
    }

    void Renderer2D::end_scene() { Flush(); }

    void Renderer2D::Flush() {
        if (s_data.quad_index_count == 0) return;
        uint32_t dataSize = (uint8_t *) s_data.quad_vertex_buffer_ptr - (uint8_t *) s_data.quad_vertex_buffer_base;
        s_data.qad_vertex_buffer->set_data(s_data.quad_vertex_buffer_base, dataSize);
        for (uint32_t i = 0; i < s_data.texture_slot_index; i++) s_data.TextureSlots[i]->bind(i);
        RenderCommand::draw_indexed(s_data.quad_vertex_array, s_data.quad_index_count);
        s_data.stats.draw_calls++; // Increment draw call count every time we flush
    }

    void
    Renderer2D::draw_quad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                          const glm::vec4 &tint_color) {
        // ... check if batch is full and needs flushing (StartBatch) ...
        if (s_data.quad_index_count >= Renderer2DData::max_indices) {
            end_scene(); // This calls Flush()
            // BeginScene(OrthographicCamera(glm::mat4(1.0f))); // TODO: We need to refactor Begin/End/Flush
        }


        constexpr size_t quad_vertex_count = 4;
        constexpr glm::vec2 texture_coords[] = {{0.0f, 0.0f},
                                                {1.0f, 0.0f},
                                                {1.0f, 1.0f},
                                                {0.0f, 1.0f}};
        constexpr glm::vec4 quad_vertex_positions[] =
                {
                        {-0.5f, -0.5f, 0.0f, 1.0f},
                        {0.5f,  -0.5f, 0.0f, 1.0f},
                        {0.5f,  0.5f,  0.0f, 1.0f},
                        {-0.5f, 0.5f,  0.0f, 1.0f}
                };

        // ... logic to find texture_slot from bound textures array ...
        // If texture is nullptr, texture_slot can be 0 (the white texture).
        float texture_slot = 0.0f; // Default to white texture
        if (texture) {
            for (uint32_t i = 1; i < s_data.texture_slot_index; i++) {
                if (*s_data.TextureSlots[i].get() == *texture.get()) {
                    texture_slot = static_cast<float>(i);
                    break;
                }
            }

            if (texture_slot == 0.0f) {
                texture_slot = static_cast<float>(s_data.texture_slot_index);
                s_data.TextureSlots[s_data.texture_slot_index] = texture;
                s_data.texture_slot_index++;
            }
        }

        for (size_t i = 0; i < quad_vertex_count; i++) {
            s_data.quad_vertex_buffer_ptr->position = transform * quad_vertex_positions[i];
            s_data.quad_vertex_buffer_ptr->color = tint_color;
            s_data.quad_vertex_buffer_ptr->tex_coord = texture_coords[i];
            s_data.quad_vertex_buffer_ptr->tex_index = texture_slot;
            s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
            s_data.quad_vertex_buffer_ptr++;
        }

        s_data.quad_index_count += 6;
        s_data.stats.quad_count++;
    }

    void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {
        // Check if the batch needs to be flushed
        if (s_data.quad_index_count >= Renderer2DData::max_indices) {
            end_scene(); // This calls Flush()
            //BeginScene(camera); // This is a problem, we don't have the camera. We need to refactor Begin/End/Flush
            // For now, let's call a new function: StartNewBatch()
        }

        glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        const float textureIndex = 0.0f; // White Texture
        const float tilingFactor = 1.0f;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[0];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[1];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[2];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[3];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

    void
    Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                         float tilingFactor, const glm::vec4 &tintColor) {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
    }

    void
    Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                         float tilingFactor, const glm::vec4 &tintColor) {

        // Check if we need to flush because we're out of indices OR texture slots
        if (s_data.quad_index_count >= Renderer2DData::max_indices ||
            s_data.texture_slot_index >= Renderer2DData::max_texture_slots) {
            end_scene(); // This calls Flush()
            // We will need to start a new batch. We need the camera from BeginScene. This highlights
            // that the Flush logic needs to be internal and potentially restart the batch.
            // For now, let's assume we don't exceed the batch limit.
        }

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_data.texture_slot_index; i++) {
            if (*s_data.TextureSlots[i].get() == *texture.get()) {
                textureIndex = (float) i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            textureIndex = (float) s_data.texture_slot_index;
            s_data.TextureSlots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[0];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[1];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[2];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[3];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_index_count += 6;

        s_data.stats.quad_count++;
    }

// Rotated Colored Quad
    void
    Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                const glm::vec4 &color) {
        // ... (check batch limit, same as DrawQuad)
        if (s_data.quad_index_count >= Renderer2DData::max_indices) {
            end_scene(); // This calls Flush()
            // BeginScene(camera); // We need the camera, but we don't have it here. Refactor needed.
        }

        const float textureIndex = 0.0f; // White Texture
        const float tilingFactor = 1.0f;

        // Calculate the full transform matrix including rotation
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        // The rest of the function is identical to DrawQuad, using this new transform matrix
        // ... (add 4 vertices to the buffer)
        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[0];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[1];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[2];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[3];
        s_data.quad_vertex_buffer_ptr->color = color;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_index_count += 6; // 6 indices for 2 triangles

        s_data.stats.quad_count++;
    }

// Rotated Textured Quad
    void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                     const glm::vec4 &tintColor) {
        // ... (check batch limit and texture index, same as textured DrawQuad)

        // Calculate the full transform matrix including rotation
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        // The rest of the function is identical to textured DrawQuad, using this new transform matrix
        // ... (add 4 vertices to the buffer)
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_data.texture_slot_index; i++) {
            if (*s_data.TextureSlots[i].get() == *texture.get()) {
                textureIndex = (float) i;
                break;
            }
        }
        if (textureIndex == 0.0f) {
            textureIndex = (float) s_data.texture_slot_index;
            s_data.TextureSlots[s_data.texture_slot_index] = texture;
            s_data.texture_slot_index++;
        }

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[0];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[1];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 0.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[2];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_positions[3];
        s_data.quad_vertex_buffer_ptr->color = tintColor;
        s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 1.0f};
        s_data.quad_vertex_buffer_ptr->tex_index = textureIndex;
        s_data.quad_vertex_buffer_ptr->tiling_factor = tilingFactor;
        s_data.quad_vertex_buffer_ptr++;

        s_data.quad_index_count += 6; // 6 indices for 2 triangles

        s_data.stats.quad_count++;
    }

// Implement the 2D overloads by calling the 3D ones
    void
    Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation,
                                const glm::vec4 &color) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                     const glm::vec4 &tintColor) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
    }

    Renderer2D::Statistics Renderer2D::get_stats() { return s_data.stats; }

    void Renderer2D::reset_stats() {
        s_data.stats.draw_calls = 0;
        s_data.stats.quad_count = 0;
    }
}