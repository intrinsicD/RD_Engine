#pragma once

#include "Shader.h"
#include "VertexArray.h"
#include <glm/glm.hpp>
#include <memory>

namespace RDE {
    class Renderer3D {
    public:
        static void Init();

        static void Shutdown();

        // Controlled by the main Renderer
        static void BeginPass();

        static void EndPass();

        static void SetShaderAndSceneUniforms(const std::shared_ptr<Shader>& shader);

        static void Submit(const std::shared_ptr<VertexArray> &vertex_array,
                           const glm::mat4 &model_transform);
    };
}