#include "GeometrySystem.h"
#include "AssetComponentTypes.h"
#include "components/TagComponent.h"
#include "core/SystemDependencyBuilder.h"

#include <entt/entity/registry.hpp>

namespace RDE{
    GeometrySystem::GeometrySystem(entt::registry &registry)
        : m_registry(registry) {
    }

    void GeometrySystem::init() {
        // Initialization logic for the geometry system
    }

    void GeometrySystem::update(float delta_time) {
        // Create GpuGeometry components from CpuGeometry components
        auto view = m_registry.view<AssetCpuGeometry, Dirty<AssetGpuGeometry>>();
        for (auto entity : view) {
            auto &geometry = view.get<AssetCpuGeometry>(entity);
            // Process geometry data, e.g., convert to GPU format
            // This is where you would typically upload the geometry to the GPU
            // and create an AssetGpuGeometry component if needed.

            //need a device reference to upload the geometry
        }
    }

    void GeometrySystem::shutdown() {
        // Cleanup logic for the geometry system
    }

    void GeometrySystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        builder.reads<HierarchySystem>();
    }
}