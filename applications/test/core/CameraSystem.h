#pragma once

#include "ISystem.h"

#include <entt/fwd.hpp>

namespace RDE{
    class CameraSystem : public ISystem{
    public:
        explicit CameraSystem(entt::registry &registry);

        void init() override;

        void shutdown() override;

        void update(float delta_time) override;
    private:

        void declare_dependencies(SystemDependencyBuilder &builder) override;
        entt::registry &m_registry;
    };

}