//systems/GpuMaterialInstanceSystem.h
#pragma once

#include "core/ISystem.h"

#include <entt/fwd.hpp>

namespace RDE{
    class GpuMaterialInstanceSystem : public ISystem {
    public:
        GpuMaterialInstanceSystem(entt::registry &registry);

        ~GpuMaterialInstanceSystem() override = default;

        void init() override;

        void update(float delta_time) override;

        void shutdown() override;

        void declare_dependencies(SystemDependencyBuilder &builder) override;

    private:
        entt::registry &m_registry;
    };
}