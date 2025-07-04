#pragma once

#include "components/BoundingVolumeComponent.h"

#include <entt/fwd.hpp>

namespace RDE::BoundingVolumeUtils{
    void SetBoundingVolumeDirty(entt::registry &registry, entt::entity entity_id);
}