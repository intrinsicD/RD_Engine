#pragma once

#include "core/ISystem.h"

#include <entt/fwd.hpp>

namespace RDE{
    class TransformSystem : public ISystem {
    public:
        explicit TransformSystem(entt::registry &registry);

        void init() override;

        void shutdown() override;

        void update(float delta_time) override;

        void declare_dependencies(SystemDependencyBuilder &builder) override;

    private:
        entt::registry &m_registry;
    };
}