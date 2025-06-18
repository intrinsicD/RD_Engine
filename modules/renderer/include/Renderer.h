// RDE_Project/modules/renderer/include/Renderer/Renderer.h
#pragma once

#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"

namespace RDE {
    class Renderer {
    public:
        static void Init();

        static void Shutdown();

        // A "scene" is defined by a camera's view and projection matrices.
        static void BeginScene(OrthographicCamera &camera);

        static void EndScene();

        // Submit geometry for drawing.
        static void Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray);

    private:
        // In the future, this will hold view-projection matrices.
        struct SceneData {
            glm::mat4 ViewProjectionMatrix;
        };

        static SceneData *m_scene_data;
    };
}
