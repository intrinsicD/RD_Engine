#pragma once

#include "core/ISystem.h"

#include <entt/fwd.hpp>

namespace RDE{
    class GeometrySystem : public ISystem {
    public:
        GeometrySystem(entt::registry &registry);

        ~GeometrySystem() override = default;

        void init() override;

        void update(float delta_time) override;

        void shutdown() override;
    private:
        void declare_dependencies(SystemDependencyBuilder &builder) override;

        entt::registry &m_registry;
    };
}