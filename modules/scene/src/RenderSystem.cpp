#include "systems/RenderSystem.h"
#include "scene/SystemDependencyBuilder.h"
#include "components/DirtyTagComponent.h"
#include "components/TransformComponent.h"
#include "components/CameraComponent.h"
#include "commands/UpdateBufferCommand.h"
#include "assets/AssetComponentTypes.h"
#include "renderer/RendererComponentTypes.h"
#include "core/Log.h"

#include <entt/entity/registry.hpp>
#include <utility>

namespace RDE {
    RenderSystem::RenderSystem(entt::registry &registry,
                               std::shared_ptr<AssetDatabase> asset_database,
                               RAL::Device *device) : m_registry(registry),
                                                      m_asset_database(std::move(asset_database)), m_device(device) {
    }

    void RenderSystem::init() {
        // Initialization logic for the render system
    }

    void RenderSystem::shutdown() {
        // Cleanup logic for the render system
    }

    void RenderSystem::update([[maybe_unused]] float delta_time) {
        // Update logic for the render system
        // This could include rendering entities, updating shaders, etc.


        //TODO remove this here because this should be handled by the GpuGeometryUpdateSystem

        // Process UpdateBufferCommand entities on the asset registry side
        {
            // This is now the ONLY view we need for this task. It's beautifully simple.
            auto &asset_registry = m_asset_database->get_registry();
            auto view = asset_registry.view<RenderGpuGeometry, Commands::UpdateBufferCommand>();

            for (auto entity: view) {
                const auto &command = view.get<Commands::UpdateBufferCommand>(entity);

                // Delegate the low-level work to the device.
                m_device->update_buffer_data(
                    command.target_buffer,
                    command.data.data(),
                    command.data.size(),
                    command.destinationOffset
                );
            }

            // These commands are ephemeral. Once executed, they are gone.
            asset_registry.clear<Commands::UpdateBufferCommand>();
        }
        // Process CopyBufferCommand entities on the asset registry side
        {
            auto &scene_registry = m_registry;
            auto view = scene_registry.view<RenderGpuGeometry, Commands::UpdateBufferCommand>();

            for (auto entity: view) {
                const auto &command = view.get<Commands::UpdateBufferCommand>(entity);

                // Delegate the low-level work to the device.
                m_device->update_buffer_data(
                    command.target_buffer,
                    command.data.data(),
                    command.data.size(),
                    command.destinationOffset
                );
            }

            // These commands are ephemeral. Once executed, they are gone.
            scene_registry.clear<Commands::UpdateBufferCommand>();
        }
    }

    void RenderSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        // This could include other systems or components that this system relies on
        builder.reads<CameraMatrices>();
        builder.reads<TransformWorld>();
    }

    void RenderSystem::pull_buffer_data_to_scene_registry(AssetID source_asset_id, entt::entity target_entity,
                                                          AttributeID attribute_id) {
        auto &asset_registry = m_asset_database->get_registry();
        auto &asset_gpu_component = asset_registry.get<RenderGpuGeometry>(source_asset_id->entity_id);
        auto it = asset_gpu_component.attribute_buffers.find(attribute_id);
        if (it == asset_gpu_component.attribute_buffers.end()) {
            /*RDE_CORE_ERROR("Attribute ID {} not found in asset GPU component for asset ID {}", attribute_id, source_asset_id);*/
            return;
        }
        if (it != asset_gpu_component.attribute_buffers.end()) {
            auto &buffer_handle = it->second;

            // Create a new buffer in the scene registry
            auto &scene_registry = m_registry;
            auto &render_gpu_geometry = scene_registry.get_or_emplace<RenderGpuGeometry>(target_entity);
            auto it = render_gpu_geometry.attribute_buffers.find(attribute_id);
            if (it == render_gpu_geometry.attribute_buffers.end()) {
                //Create a new buffer handle in the scene registry
                /*                RAL::BufferHandle buffer_handle = m_device->create_buffer(
                                        buffer_handle.get_size(),
                                        buffer_handle.get_usage_flags(),
                                        buffer_handle.get_memory_flags()
                                );*/
            }
            render_gpu_geometry.attribute_buffers[attribute_id] = buffer_handle;

            // Optionally, you can also set the vertex count and index count if needed
            render_gpu_geometry.vertex_count = asset_gpu_component.vertex_count;
            render_gpu_geometry.index_count = asset_gpu_component.index_count;
        } else {
        }
    }

    void RenderSystem::push_buffer_data_to_assets_database([[maybe_unused]] entt::entity source_entity, [[maybe_unused]] AssetID target_asset_id,
                                                           [[maybe_unused]] AttributeID attribute_id) {
    }
}
