#include "scene/SceneUtils.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/RenderableComponent.h"

namespace RDE{
    void EnsureDefaultComponents(entt::registry &registry, entt::entity entity){
        Require<TransformLocal>(registry, entity);
        Require<MaterialComponent>(registry, entity);
        Require<RenderableComponent>(registry, entity);
    }
}