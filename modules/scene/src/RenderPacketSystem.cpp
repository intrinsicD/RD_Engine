// systems/RenderPacketSystem.cpp
#include "systems/RenderPacketSystem.h"
#include "renderer/RendererComponentTypes.h"
#include "components/RenderableComponent.h"
#include "components/MaterialComponent.h"
#include "components/TransformComponent.h"
#include "scene/SystemDependencyBuilder.h"

namespace RDE {
    RenderPacketSystem::RenderPacketSystem(entt::registry &registry, AssetDatabase &asset_database, View &target_view)
        : m_registry(registry), m_asset_database(asset_database), m_target_view(target_view) {
    }

    void RenderPacketSystem::init() {
    }

    void RenderPacketSystem::shutdown() {
        // Cleanup if necessary
        m_target_view.clear(); // Clear the view to avoid stale data
    }

    void RenderPacketSystem::update([[maybe_unused]] float delta_time) {
        // 1. Clear the view from the previous frame
        m_target_view.clear();

        // 2. Create a view of all entities that have the components needed for rendering
        auto view = m_registry.view<const TransformWorld, const RenderableComponent, const MaterialComponent>();

        // 3. Iterate over the entities and create packets
        for (auto entity: view) {
            const auto &world_transform = view.get<const TransformWorld>(entity);
            const auto &renderable_comp = view.get<const RenderableComponent>(entity);
            const auto &material_comp = view.get<const MaterialComponent>(entity);

            // Basic check to ensure assets are valid
            if (!renderable_comp.is_valid() || !material_comp.is_valid()) {
                continue;
            }

            auto *gpu_geometry = m_asset_database.try_get<RenderGpuGeometry>(renderable_comp.geometry_id);
            auto *gpu_material = m_asset_database.try_get<RenderGpuMaterial>(material_comp.material_asset_id);

            if (!gpu_geometry || !gpu_material) continue;

            // Build the simplified packet
            RenderPacket &packet = m_target_view.emplace_back(); // More efficient
            packet.geometry = gpu_geometry;
            packet.material = gpu_material;
            packet.model_matrix = world_transform.matrix; // Assuming this method exists

            // 6. Add the packet to the view for this frame
            m_target_view.push_back(packet);
        }
    }

    void RenderPacketSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        builder.reads<TransformWorld>();
        builder.reads<RenderableComponent>();
        builder.reads<MaterialComponent>();
    }
}
