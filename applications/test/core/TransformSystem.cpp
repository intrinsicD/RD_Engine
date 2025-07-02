#include "TransformSystem.h"
#include "Transform.h"

namespace RDE::TransformSystem {
    void init(entt::registry &registry) {
        // Initialize the Transform system by ensuring the necessary components are present
        registry.on_construct<TransformParameters>().connect<&set_transform_dirty>();
        registry.on_update<TransformParameters>().connect<&set_transform_dirty>();
    }

    void shutdown(entt::registry &registry) {
        // Cleanup if necessary, currently nothing to do
        registry.clear<TransformParameters>();
        registry.clear<DirtyTransform>();
        registry.clear<TransformModelMatrix>();
    }

    void update_dirty_transforms(entt::registry &registry) {
        auto view = registry.view<TransformParameters, DirtyTransform>();
        for (auto entity: view) {
            auto &transform = view.get<TransformParameters>(entity);

            // Update the model matrix based on the current transform parameters
            registry.emplace_or_replace<TransformModelMatrix>(entity, get_model_matrix(transform));
        }
        registry.clear<DirtyTransform>();
    }

    void set_transform_dirty(entt::registry &registry, entt::entity entity_id) {
        if (!registry.valid(entity_id) || registry.all_of<TransformParameters>(entity_id)) {
            return;
        }
        registry.emplace_or_replace<DirtyTransform>(entity_id);
    }
}