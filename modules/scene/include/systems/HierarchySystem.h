#pragma once

#include "core/ISystem.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    class HierarchySystem : public ISystem {
    public:
        explicit HierarchySystem(entt::registry &registry);

        void init() override;

        void shutdown() override;

        void update(float delta_time) override;

    private:
        void declare_dependencies(SystemDependencyBuilder &builder) override;

        entt::registry &m_registry;
    };
}