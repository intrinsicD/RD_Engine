#include "HierarchySystem.h"
#include "Hierarchy.h"
#include "Transform.h"
#include "SystemDependencyBuilder.h"

#include <stack>

namespace RDE {
    HierarchySystem::HierarchySystem(entt::registry &registry) : m_registry(registry) {}

    void HierarchySystem::init() {

    }

    void HierarchySystem::shutdown() {
        m_registry.clear<Hierarchy>();
    }

    void HierarchySystem::update(float delta_time) {
        // Find all entities that have a Transform::Dirty and children.
        // These are the roots of the hierarchies we need to process.
        auto view = m_registry.view<Hierarchy, TransformDirty>();

        for (auto entity: view) {
            // Use a non-recursive stack to traverse the children to avoid stack overflow
            std::stack<entt::entity> stack;
            stack.push(entity);

            while (!stack.empty()) {
                const entt::entity current_parent_entity = stack.top();
                stack.pop();

                // Iterate through all direct children of the current parent
                const auto &parent_hierarchy = m_registry.get<Hierarchy>(current_parent_entity);
                entt::entity child_iter = parent_hierarchy.first_child;

                while (m_registry.valid(child_iter)) {
                    // If the child isn't already marked dirty, mark it and push to stack
                    if (!m_registry.all_of<TransformDirty>(child_iter)) {
                        m_registry.emplace<TransformDirty>(child_iter);

                        // If this child also has children, it needs to be processed
                        if (m_registry.all_of<Hierarchy>(child_iter)) {
                            stack.push(child_iter);
                        }
                    }

                    // Move to the next sibling
                    child_iter = m_registry.get<Hierarchy>(child_iter).next_sibling;
                }
            }
        }
    }

    void HierarchySystem::declare_dependencies(SystemDependencyBuilder &builder) {
        builder.reads<Hierarchy>();
        builder.reads<TransformDirty>();
        builder.writes<TransformDirty>();
    }
}
