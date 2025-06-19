#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderCommand.h"

namespace RDE {
    struct Renderer3DData {
        glm::mat4 view_projection_matrix;
    };

    static Renderer3DData s_data;

    void Renderer3D::Init() {

    }

    void Renderer3D::Shutdown() {
    }

    void Renderer3D::BeginPass() {
        RenderCommand::SetDepthTest(true);
        RenderCommand::SetBlending(false);
    }

    void Renderer3D::EndPass() {

    }

    // NOTE: This initial version of Renderer3D submits one draw call per object.
    // A more advanced implementation would use batching, sorting, and instancing.
    void Renderer3D::Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertex_array,
                            const glm::mat4 &model_transform) {
        shader->bind();
        // Get the VP matrix from the main Renderer's scene data
        shader->set_mat4("u_ViewProjection", Renderer::GetSceneData()->view_projection_matrix);
        shader->set_mat4("u_Model", model_transform);

        vertex_array->bind();
        RenderCommand::DrawIndexed(vertex_array);
    }
}