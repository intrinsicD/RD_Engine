// RDE/Core/SystemScheduler.h
#pragma once

#include "SystemGraph.h" // Your graph implementation
#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace RDE {

    class SystemScheduler {
    public:
        explicit SystemScheduler(entt::registry& registry);

        // Phase 1: Call this for each system during startup.
        template<typename T, typename... Args>
        void register_system(Args&&... args) {
            if (m_is_baked) {
                throw std::runtime_error("Cannot register new systems after the scheduler has been baked.");
            }
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->init(); // Initialize the system if needed
            m_graph.register_system(m_registry, std::move(system));
        }

        // Phase 2: Call this once after all systems are registered.
        void bake();

        // Phase 3: Call this every frame in the main loop.
        void execute(float delta_time);

        // Phase 4: Call this during application shutdown.
        void shutdown();

    private:
        entt::registry& m_registry;
        SystemGraph m_graph;
        bool m_is_baked = false;

        // The baked execution plan, stored for fast execution.
        std::vector<std::vector<ISystem*>> m_execution_stages;
    };

} // namespace RDE