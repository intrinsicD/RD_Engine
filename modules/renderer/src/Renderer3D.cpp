#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderCommand.h"

namespace RDE {
    struct Renderer3DData {
        glm::mat4 view_projection_matrix;
        std::shared_ptr<Shader> active_shader;
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
        s_data.active_shader = nullptr;
    }

    void Renderer3D::SetShaderAndSceneUniforms(const std::shared_ptr<Shader> &shader) {
        s_data.active_shader = shader;
        s_data.active_shader->bind();

        // Set scene-level uniforms ONCE
        auto scene_data = Renderer::GetSceneData();
        s_data.active_shader->set_mat4("u_ViewProjection", scene_data->view_projection_matrix);
        s_data.active_shader->set_float("u_Light.Direction", scene_data->main_light.direction);
        s_data.active_shader->set_float("u_Light.Color", scene_data->main_light.color);
    }

    // NOTE: This initial version of Renderer3D submits one draw call per object.
    // A more advanced implementation would use batching, sorting, and instancing.
    void Renderer3D::Submit(const std::shared_ptr<VertexArray> &vertex_array,
                            const glm::mat4 &model_transform) {
        RDE_CORE_ASSERT(s_data.active_shader, "Renderer3D::Submit called without an active shader! Call SetShaderAndSceneUniforms first.");

        // Set per-object uniforms
        s_data.active_shader->set_mat4("u_Model", model_transform);

        vertex_array->bind();
        RenderCommand::DrawIndexed(vertex_array);
    }
}