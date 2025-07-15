// systems/RenderPacketSystem.h

#pragma once

#include "core/ISystem.h"
#include "assets/AssetDatabase.h"
#include "renderer/RenderPacket.h"

#include <entt/entity/registry.hpp>

namespace RDE {

    class RenderPacketSystem : public ISystem {
    public:
        // The system needs access to the database to resolve AssetIDs and the view to populate it.
        RenderPacketSystem(entt::registry& registry, AssetDatabase& asset_database, View& target_view);

        void init() override;

        void shutdown() override;

        void update(float delta_time) override;
    private:

        // Declare dependencies for this system
        void declare_dependencies(SystemDependencyBuilder &builder) override;

        entt::registry& m_registry; // The registry we operate on
        AssetDatabase& m_asset_database;
        View& m_target_view; // A reference to the view we will fill
    };
}