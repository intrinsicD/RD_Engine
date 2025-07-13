#include "systems/RenderSystem.h"
#include "scene/SystemDependencyBuilder.h"
#include "components/TagComponent.h"
#include "components/TransformComponent.h"
#include "components/CameraComponent.h"
#include "assets/AssetComponentTypes.h"
#include "renderer/RendererComponentTypes.h"
#include "core/Log.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    RenderSystem::RenderSystem(entt::registry &registry,
                               std::shared_ptr<AssetDatabase> asset_database,
                               RAL::Device *device) : m_registry(registry),
                                                      m_asset_database(asset_database), m_device(device) {
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
