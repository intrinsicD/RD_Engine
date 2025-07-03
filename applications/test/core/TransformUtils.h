#pragma once

#include "Transform.h"

#include <entt/fwd.hpp>

namespace RDE::TransformUtils{
    using TransformParameters = TransformLocal;

    glm::mat4 GetModelMatrix(const TransformParameters &transform);

    TransformParameters DecomposeModelMatrix(const glm::mat4 &model_matrix);

    void SetTransformDirty(entt::registry &registry, entt::entity entity_id);
}