//systems/GpuMaterialInstanceSystem.cpp
#include "systems/GpuMaterialInstanceSystem.h"
#include "assets/AssetComponentTypes.h"
#include "components/DirtyTagComponent.h"
#include "scene/SystemDependencyBuilder.h"

#include <entt/entity/registry.hpp>

namespace RDE{
    GpuMaterialInstanceSystem::GpuMaterialInstanceSystem(entt::registry &registry)
            : m_registry(registry) {
    }

    void GpuMaterialInstanceSystem::init() {
        // Initialization logic for the geometry system
    }

    void GpuMaterialInstanceSystem::update(float delta_time) {
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

    void GpuMaterialInstanceSystem::shutdown() {
        // Cleanup logic for the geometry system
    }

    void GpuMaterialInstanceSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        builder.reads<HierarchySystem>();
    }
}