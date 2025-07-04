#include "HierarchyUtils.h"
#include "components/HierarchyComponent.h"

#include <entt/entity/registry.hpp>

namespace RDE::HierarchyUtils{
    void SetParent(entt::registry &registry, entt::entity child_entity, entt::entity parent_entity) {
        if (child_entity == parent_entity || !registry.valid(child_entity)) {
            return; // Can't parent to self or if invalid
        }

        // Ensure both entities have the hierarchy component
        if (!registry.all_of<Hierarchy>(child_entity)) {
            registry.emplace<Hierarchy>(child_entity);
        }
        if (registry.valid(parent_entity) && !registry.all_of<Hierarchy>(parent_entity)) {
            registry.emplace<Hierarchy>(parent_entity);
        }

        // First, detach from any existing parent
        RemoveParent(registry, child_entity);

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

    void RemoveParent(entt::registry &registry, entt::entity child_entity) {
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
}