// RDE_Project/modules/core/src/Entity.cpp
#include "Core/Entity.h"
namespace RDE {
    Entity::Entity(entt::entity handle, Scene *scene)
            : m_handle(handle), m_scene(scene) {
    }
}