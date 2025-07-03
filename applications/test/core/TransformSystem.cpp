#include "TransformSystem.h"
#include "TransformUtils.h"
#include "BoundingVolumeUtils.h"
#include "CameraUtils.h"
#include "Hierarchy.h"
#include "SystemDependencyBuilder.h"

#include <entt/entity/registry.hpp>
#include <stack>

namespace RDE {
    namespace Detail {
        inline void set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
            registry.emplace_or_replace<TransformDirty>(entity_id);
        }
    }

    TransformSystem::TransformSystem(entt::registry &registry) : m_registry(registry) {

    }

    void TransformSystem::init() {
        // Initialize the Transform system by ensuring the necessary components are present
        m_registry.on_construct<TransformLocal>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_update<TransformLocal>().connect<&Detail::set_dirty_on_change>();
    }

    void TransformSystem::shutdown() {
        // Cleanup if necessary, currently nothing to do
        m_registry.clear<TransformLocal>();
        m_registry.clear<TransformWorld>();
        m_registry.clear<TransformDirty>();
    }

    bool IsRootOfDirtyTree(entt::registry &registry, entt::entity entity_id) {
        if (auto *hierarchy = registry.try_get<Hierarchy>(entity_id)) {
            // The entity is part of a hierarchy, check its parent.
            if (registry.valid(hierarchy->parent)) {
                // If the parent is valid AND also dirty, then we are NOT the root.
                // The parent will process us when it gets its turn in this loop.
                if (registry.all_of<TransformDirty>(hierarchy->parent)) {
                    return false;
                }
            }
        }
        return true;
    }

    void TransformSystem::update(float delta_time) {
        auto view = m_registry.view<TransformLocal, TransformDirty>();

        for (auto entity : view) {
            // We must check if the entity is still dirty, as it might have been
            // processed already as a child of another dirty root.
            if (!m_registry.all_of<TransformDirty>(entity)) {
                continue;
            }

            if (IsRootOfDirtyTree(m_registry, entity)) {
                // --- This is the new, combined logic ---
                std::stack<entt::entity> stack;
                stack.push(entity);

                while (!stack.empty()) {
                    entt::entity current_entity = stack.top();
                    stack.pop();

                    // --- 1. Calculate the matrix for the CURRENT node ---
                    const auto& local_transform = m_registry.get<TransformLocal>(current_entity);
                    glm::mat4 local_matrix = TransformUtils::GetModelMatrix(local_transform);

                    glm::mat4 parent_world_matrix = glm::mat4(1.0f);
                    if (auto* hierarchy = m_registry.try_get<Hierarchy>(current_entity)) {
                        if (m_registry.valid(hierarchy->parent)) {
                            // We can safely get this, as parents are always processed before children.
                            parent_world_matrix = m_registry.get<TransformWorld>(hierarchy->parent).matrix;
                        }
                    }

                    // Set the final world matrix for the current node
                    auto& world_transform = m_registry.get_or_emplace<TransformWorld>(current_entity);
                    world_transform.matrix = parent_world_matrix * local_matrix;

                    // --- 2. Push its children onto the stack to be processed next ---
                    if (auto* hierarchy = m_registry.try_get<Hierarchy>(current_entity)) {
                        entt::entity child_iter = hierarchy->first_child;
                        while (m_registry.valid(child_iter)) {
                            stack.push(child_iter);
                            child_iter = m_registry.get<Hierarchy>(child_iter).next_sibling;
                        }
                    }
                }
            }
        }

        // --- Dependency Propagation: AFTER all calculations are complete ---
        // At this point, all WorldTransform matrices are up-to-date for this frame.
        // It is now safe to tell other systems to update.
        for (auto entity : view) {
            BoundingVolumeUtils::SetBoundingVolumeDirty(m_registry, entity);
            CameraUtils::SetCameraDirty(m_registry, entity);
        }

        // --- Final Cleanup ---
        // Now that all work related to dirty transforms is done, clear the flags.
        m_registry.clear<TransformDirty>();
    }

    void TransformSystem::declare_dependencies(RDE::SystemDependencyBuilder &builder) {
        builder.reads<TransformLocal>();
        builder.reads<TransformDirty>();

        builder.writes<TransformDirty>();
        builder.writes<TransformWorld>();
        builder.writes<BoundingVolumeDirty>();
        builder.writes<CameraComponent>();
    }
}
