#include "CameraSystem.h"
#include "CameraUtils.h"
#include "SystemDependencyBuilder.h"

#include "Transform.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    namespace Detail {
        void inline set_dirty_on_change(entt::registry &registry, entt::entity entity_id) {
            registry.emplace_or_replace<CameraDirty>(entity_id);
        }
        inline void require_transform(entt::registry &registry, entt::entity entity_id) {
            registry.get_or_emplace<TransformLocal>(entity_id);
        }
    }

    CameraSystem::CameraSystem(entt::registry &registry) : m_registry(registry) {}

    void CameraSystem::init() {
        m_registry.on_construct<CameraProjectionParameters>().connect<&Detail::set_dirty_on_change>();
        m_registry.on_construct<CameraProjectionParameters>().connect<&Detail::require_transform>();

        m_registry.on_update<CameraProjectionParameters>().connect<&Detail::set_dirty_on_change>();

        auto default_camera_entity = CameraUtils::CreateCameraEntity(m_registry);
        CameraUtils::MakeCameraEntityPrimary(m_registry, default_camera_entity);
    }

    void CameraSystem::shutdown() {
        // Cleanup the camera system, if needed
        // This could include removing cameras or other cleanup tasks
        m_registry.clear<CameraViewParameters>();
        m_registry.clear<CameraProjectionParameters>();
        m_registry.clear<CameraMatrices>();
        m_registry.clear<CameraDirty>();
        m_registry.clear<CameraPrimary>();
    }

    void CameraSystem::update(float delta_time) {
        // Query for all dirty cameras that have the necessary components
        auto view = m_registry.view<CameraProjectionParameters, TransformWorld, CameraDirty>();

        for (auto entity: view) {
            const auto &proj_params = view.get<CameraProjectionParameters>(entity);
            const auto &world = view.get<TransformWorld>(entity);
            glm::mat4 view_matrix = CameraUtils::CalculateViewMatrixFromModelMatrix(world.matrix);
            glm::mat4 proj_matrix = CameraUtils::CalculateProjectionMatrix(proj_params);
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
