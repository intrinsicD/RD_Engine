#include "scene/Scene.h"
#include "assets/AssetComponentTypes.h"
#include "components/DirtyTagComponent.h"
#include "components/NameTagComponent.h"
#include "components/TransformComponent.h"

namespace RDE {
    bool Scene::instantiate(entt::entity entity_id, AssetID prefab_root_id) {
        auto &asset_registry = m_asset_database->get_registry();
        auto &scene_registry = *m_registry;

        // 1. Get the list of all template entities from the root's AssetPrefab component.
        auto *prefab_comp = asset_registry.try_get<AssetPrefab>(prefab_root_id->entity_id);
        if (!prefab_comp) return false;

        std::unordered_map<AssetID, entt::entity> template_to_scene_map;
        entt::entity scene_root_entity = entt::null;

        // 2. Clone each template entity into the scene.
        for (const auto &template_asset_id: prefab_comp->template_entities) {
            auto template_entity = template_asset_id->entity_id;
            auto new_scene_entity = scene_registry.create();
            template_to_scene_map[template_asset_id] = new_scene_entity;

            if (template_asset_id == prefab_root_id) {
                scene_root_entity = new_scene_entity;
            }

            // --- CLONE COMPONENTS ---
            // This is the magic. Iterate over all components on the template entity
            // and copy them to the new scene entity. EnTT doesn't have a built-in "clone"
            // so we do it manually. A registry "snapshot" could also work here.

            // Clone Tag/Name
            if (auto *name = asset_registry.try_get<AssetName>(template_entity)) {
                scene_registry.emplace<TagComponent>(new_scene_entity, name->name);
            }
            // Clone Transform
            if (auto *transform = asset_registry.try_get<AssetTransformLocal>(template_entity)) {
                scene_registry.emplace<TransformLocal>(new_scene_entity, *transform); // Just copy it
            }
            // Clone Renderable
            if (auto *asset_renderable = asset_registry.try_get<AssetRenderable>(template_entity)) {
                // Convert AssetRenderable to the scene's RenderableComponent
                scene_registry.emplace<RenderableComponent>(new_scene_entity, asset_renderable->mesh_asset,
                                                        asset_renderable->material_asset);
            }
            // ... Clone other components ...
        }

        // 3. Re-create the hierarchy in the scene
        for (const auto &template_asset_id: prefab_comp->template_entities) {
            auto template_entity = template_asset_id->entity_id;
            auto *hierarchy_comp = asset_registry.try_get<AssetTransformLocal>(template_entity);
            if (hierarchy_comp && hierarchy_comp->parent.is_valid()) {
                entt::entity child_scene_entity = template_to_scene_map.at(template_asset_id);
                entt::entity parent_scene_entity = template_to_scene_map.at(hierarchy_comp->parent);
                setParent(child_scene_entity, parent_scene_entity);
            }
        }
    }
}
