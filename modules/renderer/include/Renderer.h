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

        // A "scene" is defined by a camera's view and projection matrices.
        static void BeginScene(const Camera &camera, const glm::mat4 &camera_transform);

        static void EndScene();

        // Submit geometry for drawing.
        static void Submit(const std::shared_ptr<Shader> &shader,
                           const std::shared_ptr<VertexArray> &vertex_array,
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

        // In the future, this will hold view-projection matrices.
        struct SceneData {
            glm::mat4 view_projection_matrix;
        };

        static SceneData *GetSceneData();
    };
}
