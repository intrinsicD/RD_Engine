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

        void declare_dependencies(SystemDependencyBuilder &builder) override;

    private:
        entt::registry &m_registry;
    };
}