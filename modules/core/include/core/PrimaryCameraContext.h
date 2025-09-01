#pragma once

#include "components/CameraComponent.h"
#include "components/TransformComponent.h"

#include <entt/entity/entity.hpp>

namespace RDE {
    struct PrimaryCameraContext {
        entt::entity entity = entt::null;

        struct DefaultCameraConfig {
            CameraProjectionParameters projection_parameters;
            TransformLocal default_transform;
        } default_camera_config;
    };
}
