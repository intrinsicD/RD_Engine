#include "RenderSystem.h"
#include "SystemDependencyBuilder.h"
#include "Transform.h"
#include "Camera.h"

#include <entt/entity/registry.hpp>

namespace RDE{
    RenderSystem::RenderSystem(entt::registry &registry) : m_registry(registry){

    }

    void RenderSystem::init() {
        // Initialization logic for the render system
    }

    void RenderSystem::shutdown() {
        // Cleanup logic for the render system
    }

    void RenderSystem::update(float delta_time) {
        // Update logic for the render system
        // This could include rendering entities, updating shaders, etc.

    }

    void RenderSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        // This could include other systems or components that this system relies on
        builder.reads<CameraMatrices>();
        builder.reads<TransformWorld>();
    }
}