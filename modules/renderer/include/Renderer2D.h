// RDE_Project/modules/renderer/include/Renderer/Renderer2D.h
#pragma once

#include "OrthographicCamera.h"
#include "VertexArray.h"
#include "Texture.h"

#include <glm/glm.hpp>
namespace RDE {
    class Renderer2D {
    public:
        static void Init();

        static void Shutdown();

        static void begin_scene(const OrthographicCamera &camera);

        static void end_scene();

        static void Flush();

        // Primitives
        static void draw_quad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color);

        static void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void
        DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                 float tilingFactor = 1.0f, const glm::vec4 &tintColor = glm::vec4(1.0f));

        static void
        DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                 float tilingFactor = 1.0f, const glm::vec4 &tintColor = glm::vec4(1.0f));

        static void
        DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);

        static void
        DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);

        static void DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation,
                                    const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                                    const glm::vec4 &tintColor = glm::vec4(1.0f));

        static void DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                    const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                                    const glm::vec4 &tintColor = glm::vec4(1.0f));

        struct Statistics {
            uint32_t draw_calls = 0;
            uint32_t quad_count = 0;

            uint32_t get_total_vertex_count() const { return quad_count * 4; }

            uint32_t get_total_index_count() const { return quad_count * 6; }
        };

        static Statistics get_stats();

        static void reset_stats();
    };
}