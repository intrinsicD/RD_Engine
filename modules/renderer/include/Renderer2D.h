#pragma once

#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>

namespace RDE {
    class Renderer2D {
    public:
        static void Init();

        static void Shutdown();

        // New lifecycle controlled by the main Renderer
        static void BeginPass();

        static void EndPass();

        // --- Stats ---
        struct Statistics {
            uint32_t draw_calls = 0;
            uint32_t quad_count = 0;

            uint32_t get_total_vertex_count() const { return quad_count * 4; }

            uint32_t get_total_index_count() const { return quad_count * 6; }
        };

        static Statistics GetStats();

        static void ResetStats();

        // --- Primitives API ---

        // Base primitive using a full transform matrix
        static void DrawQuad(const glm::mat4 &transform, const glm::vec4 &color);

        static void
        DrawQuad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture, float tiling_factor = 1.0f,
                 const glm::vec4 &tint_color = glm::vec4(1.0f));

        // Convenience overloads using position and size
        static void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void
        DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                 float tiling_factor = 1.0f, const glm::vec4 &tint_color = glm::vec4(1.0f));

        static void
        DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                 float tiling_factor = 1.0f, const glm::vec4 &tint_color = glm::vec4(1.0f));

        // Convenience overloads for rotated quads
        static void DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                    const glm::vec4 &color);

        static void DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation_radians,
                                    const glm::vec4 &color);

        static void DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                    const std::shared_ptr<Texture2D> &texture, float tiling_factor = 1.0f,
                                    const glm::vec4 &tint_color = glm::vec4(1.0f));

        static void DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation_radians,
                                    const std::shared_ptr<Texture2D> &texture, float tiling_factor = 1.0f,
                                    const glm::vec4 &tint_color = glm::vec4(1.0f));

    private:
        // Internal batch management functions
        static void StartBatch();

        static void NextBatch();
    };
}