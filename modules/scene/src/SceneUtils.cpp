#include "scene/SceneUtils.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/RenderableComponent.h"

namespace RDE{
    void EnsureDefaultComponents(entt::registry &registry, entt::entity entity){
        Requires<TransformLocal>(registry, entity);
        Requires<MaterialComponent>(registry, entity);
        Requires<RenderableComponent>(registry, entity);
    }
}