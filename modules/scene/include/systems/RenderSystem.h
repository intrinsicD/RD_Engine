#pragma once

#include "core/ISystem.h"
#include "core/AttributeRegistry.h"
#include "assets/AssetDatabase.h"

#include "ral/Device.h"

#include <entt/fwd.hpp>

namespace RDE {
    class RenderSystem : public ISystem {
    public:
        RenderSystem(entt::registry &, std::shared_ptr<AssetDatabase>, RAL::Device *device);

        void init() override;

        void shutdown() override;

        void update(float delta_time) override;

    private:
        void declare_dependencies(SystemDependencyBuilder &builder) override;

        AttributeRegistry attribute_registry;
        RAL::Device *m_device;
        entt::registry &m_registry;
        std::shared_ptr<AssetDatabase> m_asset_database;
    };
}