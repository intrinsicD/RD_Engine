// RDE_Project/modules/renderer/src/Renderer.cpp
#include "../include/Renderer.h"
namespace RDE {
    Renderer::SceneData *Renderer::m_scene_data = new Renderer::SceneData;

    void Renderer::Init() {
        // Initialize render commands, enable depth testing, etc.
        RenderCommand::init();
    }

    void Renderer::Shutdown() {
        delete m_scene_data;
    }

    void Renderer::BeginScene(OrthographicCamera &camera) {
        m_scene_data->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
    }

    void Renderer::EndScene() {
    }

    void Renderer::Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray) {
        shader->bind();
        // Upload the matrix from the scene data to the shader.
        shader->set_mat4("u_ViewProjection", m_scene_data->ViewProjectionMatrix);

        vertexArray->bind();
        RenderCommand::draw_indexed(vertexArray);
    }
}