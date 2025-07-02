#pragma once

#include <entt/entity/registry.hpp>

namespace RDE::CameraSystem{
    void init(entt::registry &registry);

    void shutdown(entt::registry &registry);

    void update_dirty_cameras(entt::registry &registry);

    entt::entity create_camera_entity(entt::registry &registry, entt::entity entity_id = entt::null);

    bool make_camera_entity_primary(entt::registry &registry, entt::entity entity_id);

    entt::entity get_primary_camera(entt::registry &registry);

    void set_camera_dirty(entt::registry &registry, entt::entity entity_id);
}