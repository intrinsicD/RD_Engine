// RDE_Project/modules/renderer/src/Renderer.cpp
#include "Renderer.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

namespace RDE {
    // The Renderer now holds the scene data that was in Renderer3D
    static Renderer::SceneData s_scene_data;

    void Renderer::Init() {
        // Initialize render commands, enable depth testing, etc.
        RenderCommand::Init();
        Renderer2D::Init(); // The batch renderer still needs its own init
        Renderer3D::Init(); // 3D renderer might need init later too
    }

    void Renderer::Shutdown() {
        Renderer2D::Shutdown();
        Renderer3D::Shutdown();
    }

    Renderer::SceneData *Renderer::GetSceneData() {
        return &s_scene_data;
    }

    void Renderer::BeginScene(const Camera &camera, const glm::mat4 &camera_transform) {
        glm::mat4 view_matrix = glm::inverse(camera_transform);
        s_scene_data.view_projection_matrix = camera.get_projection_matrix() * view_matrix;

        // Start the specialized renderers' batches/passes
        // This is where we explicitly manage state changes!
        Renderer3D::BeginPass();
        Renderer2D::BeginPass();
    }

    void Renderer::EndScene() {
        // Execute the draw calls
        Renderer3D::EndPass(); // Flushes 3D commands
        Renderer2D::EndPass(); // Flushes 2D commands (UI on top)
    }

    void Renderer::Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertex_array,
                          const glm::mat4 &model_transform) {
        Renderer3D::Submit(shader, vertex_array, model_transform);
    }

    void Renderer::DrawScreenSpaceQuad(const glm::mat4 &transform, const glm::vec4 &color) {
        Renderer2D::DrawQuad(transform, color);
    }

    void Renderer::DrawScreenSpaceQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
        Renderer2D::DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer::DrawScreenSpaceQuad(const glm::vec2 &position, const glm::vec2 &size,
                                       const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                                       const glm::vec4 &tint_color) {
        Renderer2D::DrawQuad({position.x, position.y, 0.0f}, size, texture, tiling_factor, tint_color);
    }

    void Renderer::DrawScreenSpaceRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                              const glm::vec4 &color) {
        Renderer2D::DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation_radians, color);
    }

    void Renderer::DrawScreenSpaceRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation_radians,
                                              const std::shared_ptr<Texture2D> &texture, float tiling_factor,
                                              const glm::vec4 &tint_color) {
        Renderer2D::DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation_radians, texture, tiling_factor,
                                    tint_color);
    }
}