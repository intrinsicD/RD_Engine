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

        void declare_dependencies(SystemDependencyBuilder &builder) override;

    private:
        void pull_buffer_data_to_scene_registry(AssetID source_asset_id, entt::entity target_entity, AttributeID attribute_id);

        void push_buffer_data_to_assets_database(entt::entity source_entity, AssetID target_asset_id, AttributeID attribute_id);

        AttributeRegistry attribute_registry;
        entt::registry &m_registry;
        std::shared_ptr<AssetDatabase> m_asset_database;
        RAL::Device *m_device;
    };
}