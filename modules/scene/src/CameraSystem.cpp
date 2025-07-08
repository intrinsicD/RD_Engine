#include "systems/CameraSystem.h"
#include "scene/SystemDependencyBuilder.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    namespace Detail {
        void inline set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
            registry.emplace_or_replace<CameraDirty>(entity_id);
        }

        inline void require_transform(entt::registry &registry, entt::entity entity_id) {
            if (!registry.all_of<TransformLocal>(entity_id)) {
                registry.emplace<TransformLocal>(entity_id);
            }
        }
    }

    CameraSystem::CameraSystem(entt::registry &registry) : m_registry(registry) {
    }

    void CameraSystem::init() {
        m_registry.on_construct<CameraComponent>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_construct<CameraComponent>().connect<&Detail::require_transform>();

        m_registry.on_update<CameraComponent>().connect<&Detail::set_dirty_on_change>();

        auto default_camera_entity = CameraUtils::CreateCameraEntity(m_registry);
        CameraUtils::MakeCameraEntityPrimary(m_registry, default_camera_entity);
    }

    void CameraSystem::shutdown() {
        // Cleanup the camera system, if needed
        // This could include removing cameras or other cleanup tasks
        m_registry.clear<CameraComponent>();
        m_registry.clear<CameraMatrices>();
        m_registry.clear<CameraDirty>();
        m_registry.clear<CameraPrimary>();
    }

    void CameraSystem::update(float delta_time) {
        // Query for all dirty cameras that have the necessary components
        auto view = m_registry.view<CameraComponent, TransformWorld, CameraDirty>();

        for (auto entity: view) {
            const auto &camera = view.get<CameraComponent>(entity);
            const auto &world = view.get<TransformWorld>(entity);
            glm::mat4 view_matrix = CameraUtils::CalculateViewMatrixFromModelMatrix(world.matrix);
            glm::mat4 proj_matrix = CameraUtils::CalculateProjectionMatrix(camera.projection_params);
            m_registry.emplace_or_replace<CameraMatrices>(entity, view_matrix, proj_matrix);
        }

        m_registry.clear<CameraDirty>();
    }

    void CameraSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        builder.reads<CameraProjectionParameters>();
        builder.reads<TransformWorld>();
        builder.reads<CameraDirty>();
        builder.writes<CameraMatrices>();
        builder.writes<CameraDirty>();
    }
}
