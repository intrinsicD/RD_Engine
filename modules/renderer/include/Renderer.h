// RDE_Project/modules/renderer/include/Renderer/Renderer.h
#pragma once

#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>

namespace RDE {
    class Renderer {
    public:
        static void Init();

        static void Shutdown();

        static void BeginScene(const glm::mat4 &view_projection_matrix);

        static void EndScene();

        static void Begin2DPass();

        static void End2DPass();

        static void Begin3DPass();

        static void End3DPass();

        // Submit geometry for drawing.
        static void Submit(const std::shared_ptr<VertexArray> &vertex_array,
                           const glm::mat4 &model_transform = glm::mat4(1.0f));

        // Quads
        static void DrawScreenSpaceQuad(const glm::mat4 &transform, const glm::vec4 &color);

        static void DrawScreenSpaceQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void
        DrawScreenSpaceQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture,
                            float tiling_factor = 1.0f, const glm::vec4 &tint_color = {1.0f, 1.0f, 1.0f, 1.0f});

        // Rotated Quads
        static void DrawScreenSpaceRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                               const glm::vec4 &color);

        static void DrawScreenSpaceRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                               const std::shared_ptr<Texture2D> &texture, float tiling_factor = 1.0f,
                                               const glm::vec4 &tint_color = {1.0f, 1.0f, 1.0f, 1.0f});

        // Event handling
        static void OnWindowResize(uint32_t width, uint32_t height);

        // In the future, this will hold view-projection matrices.
        struct Light {
            glm::vec3 direction{-0.5f, -0.5f, -0.5f}; // A simple directional light
            glm::vec3 color{1.0f, 1.0f, 1.0f};
        };

        struct SceneData {
            glm::mat4 view_projection_matrix;
            Light main_light;
        };

        static SceneData *GetSceneData();
    };
}
