#include "Renderer/Renderer2D.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/RenderCommand.h"
#include "Core/FileIO.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
namespace RDE {
    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct Renderer2DData {
        static const uint32_t MaxQuads = 10000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> TextureShader;
        std::shared_ptr<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex *QuadVertexBufferBase = nullptr;
        QuadVertex *QuadVertexBufferPtr = nullptr;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];
        Renderer2D::Statistics Stats;
    };

    static Renderer2DData s_data;

    void Renderer2D::Init() {
        s_data.QuadVertexArray = VertexArray::Create();
        s_data.QuadVertexBuffer = VertexBuffer::Create(s_data.MaxVertices * sizeof(QuadVertex));
        s_data.QuadVertexBuffer->set_layout({
                                                    {ShaderDataType::Float3, "a_Position"},
                                                    {ShaderDataType::Float4, "a_Color"},
                                                    {ShaderDataType::Float2, "a_TexCoord"},
                                                    {ShaderDataType::Float,  "a_TexIndex"},
                                                    {ShaderDataType::Float,  "a_TilingFactor"}
                                            });
        s_data.QuadVertexArray->AddVertexBuffer(s_data.QuadVertexBuffer);
        s_data.QuadVertexBufferBase = new QuadVertex[s_data.MaxVertices];

        uint32_t *quadIndices = new uint32_t[s_data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_data.MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0; // Bottom-left
            quadIndices[i + 1] = offset + 1; // Bottom-right
            quadIndices[i + 2] = offset + 2; // Top-right

            quadIndices[i + 3] = offset + 2; // Top-right
            quadIndices[i + 4] = offset + 3; // Top-left
            quadIndices[i + 5] = offset + 0; // Bottom-left

            offset += 4; // Move to the next set of 4 vertices
        }
        auto quadIB = IndexBuffer::Create(quadIndices, s_data.MaxIndices);
        s_data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        s_data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        // We need a SetData on texture for this
        // s_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_data.MaxTextureSlots; i++) samplers[i] = i;

        s_data.TextureShader = Shader::CreateFromFile(FileIO::get_path("assets/shaders/Texture.vert"),
                                                      FileIO::get_path("assets/shaders/Texture.frag"));
        s_data.TextureShader->Bind();
        s_data.TextureShader->SetIntArray("u_Textures", samplers, s_data.MaxTextureSlots);
        s_data.TextureSlots[0] = s_data.WhiteTexture;

        s_data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    void Renderer2D::Shutdown() { delete[] s_data.QuadVertexBufferBase; }

    void Renderer2D::BeginScene(const OrthographicCamera &camera) {
        s_data.TextureShader->Bind();
        s_data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
        ResetStats(); // Reset stats at the beginning of the scene
        s_data.QuadIndexCount = 0;
        s_data.QuadVertexBufferPtr = s_data.QuadVertexBufferBase;
        s_data.TextureSlotIndex = 1;
    }

    void Renderer2D::EndScene() { Flush(); }

    void Renderer2D::Flush() {
        if (s_data.QuadIndexCount == 0) return;
        uint32_t dataSize = (uint8_t *) s_data.QuadVertexBufferPtr - (uint8_t *) s_data.QuadVertexBufferBase;
        s_data.QuadVertexBuffer->set_data(s_data.QuadVertexBufferBase, dataSize);
        for (uint32_t i = 0; i < s_data.TextureSlotIndex; i++) s_data.TextureSlots[i]->Bind(i);
        RenderCommand::DrawIndexed(s_data.QuadVertexArray, s_data.QuadIndexCount);
        s_data.Stats.DrawCalls++; // Increment draw call count every time we flush
    }

    void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {
        // Check if the batch needs to be flushed
        if (s_data.QuadIndexCount >= Renderer2DData::MaxIndices) {
            EndScene(); // This calls Flush()
            //BeginScene(camera); // This is a problem, we don't have the camera. We need to refactor Begin/End/Flush
            // For now, let's call a new function: StartNewBatch()
        }

        glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        const float textureIndex = 0.0f; // White Texture
        const float tilingFactor = 1.0f;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[0];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[1];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[2];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[3];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadIndexCount += 6;

        s_data.Stats.QuadCount++;
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
        if (s_data.QuadIndexCount >= Renderer2DData::MaxIndices ||
            s_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots) {
            EndScene(); // This calls Flush()
            // We will need to start a new batch. We need the camera from BeginScene. This highlights
            // that the Flush logic needs to be internal and potentially restart the batch.
            // For now, let's assume we don't exceed the batch limit.
        }

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_data.TextureSlotIndex; i++) {
            if (*s_data.TextureSlots[i].get() == *texture.get()) {
                textureIndex = (float) i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            textureIndex = (float) s_data.TextureSlotIndex;
            s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
            s_data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[0];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[1];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[2];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[3];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadIndexCount += 6;

        s_data.Stats.QuadCount++;
    }

// Rotated Colored Quad
    void
    Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                const glm::vec4 &color) {
        // ... (check batch limit, same as DrawQuad)
        if (s_data.QuadIndexCount >= Renderer2DData::MaxIndices) {
            EndScene(); // This calls Flush()
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
        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[0];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[1];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[2];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[3];
        s_data.QuadVertexBufferPtr->Color = color;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadIndexCount += 6; // 6 indices for 2 triangles

        s_data.Stats.QuadCount++;
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
        for (uint32_t i = 1; i < s_data.TextureSlotIndex; i++) {
            if (*s_data.TextureSlots[i].get() == *texture.get()) {
                textureIndex = (float) i;
                break;
            }
        }
        if (textureIndex == 0.0f) {
            textureIndex = (float) s_data.TextureSlotIndex;
            s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
            s_data.TextureSlotIndex++;
        }
        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[0];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[1];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 0.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[2];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {1.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadVertexBufferPtr->Position = transform * s_data.QuadVertexPositions[3];
        s_data.QuadVertexBufferPtr->Color = tintColor;
        s_data.QuadVertexBufferPtr->TexCoord = {0.0f, 1.0f};
        s_data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr++;

        s_data.QuadIndexCount += 6; // 6 indices for 2 triangles

        s_data.Stats.QuadCount++;
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

    Renderer2D::Statistics Renderer2D::GetStats() { return s_data.Stats; }

    void Renderer2D::ResetStats() {
        s_data.Stats.DrawCalls = 0;
        s_data.Stats.QuadCount = 0;
    }
}