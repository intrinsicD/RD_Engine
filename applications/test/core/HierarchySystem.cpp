#include "HierarchySystem.h"
#include "Hierarchy.h"
#include "Transform.h"

#include <stack>

namespace RDE::HierarchySystem {
    void init(entt::registry &registry) {
    }

    void shutdown(entt::registry &registry) {
        registry.clear<Hierarchy>();
    }

    void set_parent(entt::registry &registry, entt::entity child_entity, entt::entity parent_entity) {
        if (child_entity == parent_entity || !registry.valid(child_entity)) {
            return; // Can't parent to self or if invalid
        }

        // Ensure both entities have the hierarchy component
        registry.get_or_emplace<Hierarchy>(child_entity);
        if (registry.valid(parent_entity)) {
            registry.get_or_emplace<Hierarchy>(parent_entity);
        }

        // First, detach from any existing parent
        remove_parent(registry, child_entity);

        // If the new parent is null, we are done.
        if (!registry.valid(parent_entity)) {
            return;
        }

        auto &child_hierarchy = registry.get<Hierarchy>(child_entity);
        auto &parent_hierarchy = registry.get<Hierarchy>(parent_entity);

        child_hierarchy.parent = parent_entity; // Set the back-pointer

        // Add the child to the new parent's list of children
        if (parent_hierarchy.first_child == entt::null) {
            // This is the first child
            parent_hierarchy.first_child = child_entity;
            parent_hierarchy.last_child = child_entity;
        } else {
            // Add to the end of the list
            auto &last_child_hierarchy = registry.get<Hierarchy>(parent_hierarchy.last_child);
            last_child_hierarchy.next_sibling = child_entity;
            child_hierarchy.prev_sibling = parent_hierarchy.last_child;
            parent_hierarchy.last_child = child_entity;
        }
        parent_hierarchy.num_children++;
    }

    void remove_parent(entt::registry &registry, entt::entity child_entity) {
        auto &child_hierarchy = registry.get<Hierarchy>(child_entity);
        const entt::entity parent_entity = child_hierarchy.parent;

        if (!registry.valid(parent_entity)) {
            return; // Already has no parent
        }

        auto &parent_hierarchy = registry.get<Hierarchy>(parent_entity);
        const entt::entity prev_sibling_entity = child_hierarchy.prev_sibling;
        const entt::entity next_sibling_entity = child_hierarchy.next_sibling;

        if (registry.valid(prev_sibling_entity)) {
            registry.get<Hierarchy>(prev_sibling_entity).next_sibling = next_sibling_entity;
        } else {
            // This was the first child, so update parent's first_child pointer
            parent_hierarchy.first_child = next_sibling_entity;
        }

        if (registry.valid(next_sibling_entity)) {
            registry.get<Hierarchy>(next_sibling_entity).prev_sibling = prev_sibling_entity;
        } else {
            // This was the last child, so update parent's last_child pointer
            parent_hierarchy.last_child = prev_sibling_entity;
        }

        parent_hierarchy.num_children--;
        child_hierarchy.parent = entt::null;
        child_hierarchy.prev_sibling = entt::null;
        child_hierarchy.next_sibling = entt::null;
    }

    void update_dirty_hierarchy(entt::registry &registry) {
        // Find all entities that have a Transform::Dirty and children.
        // These are the roots of the hierarchies we need to process.
        auto view = registry.view<Hierarchy, Transform::Dirty>();

        for (auto entity: view) {
            // Use a non-recursive stack to traverse the children to avoid stack overflow
            std::stack<entt::entity> stack;
            stack.push(entity);

            while (!stack.empty()) {
                const entt::entity current_parent_entity = stack.top();
                stack.pop();

                // Iterate through all direct children of the current parent
                const auto &parent_hierarchy = registry.get<Hierarchy>(current_parent_entity);
                entt::entity child_iter = parent_hierarchy.first_child;

                while (registry.valid(child_iter)) {
                    // If the child isn't already marked dirty, mark it and push to stack
                    if (!registry.all_of<Transform::Dirty>(child_iter)) {
                        registry.emplace<Transform::Dirty>(child_iter);

                        // If this child also has children, it needs to be processed
                        if (registry.all_of<Hierarchy>(child_iter)) {
                            stack.push(child_iter);
                        }
                    }

                    // Move to the next sibling
                    child_iter = registry.get<Hierarchy>(child_iter).next_sibling;
                }
            }
        }
    }
}
