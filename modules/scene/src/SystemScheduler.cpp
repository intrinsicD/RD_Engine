// RDE/Core/SystemScheduler.cpp
#include "scene/SystemScheduler.h"

namespace RDE {

    SystemScheduler::SystemScheduler(entt::registry& registry)
            : m_registry(registry) {
        // The constructor takes a reference to the central registry.
    }

    void SystemScheduler::bake() {
        if (m_is_baked) return;

        // Build the execution stages ONCE and store them.
        m_execution_stages = m_graph.build_execution_stages();
        m_is_baked = true;

        // Optional: Call init() on all systems *after* baking.
        // This allows systems to cache groups, etc.
        for (const auto& stage : m_execution_stages) {
            for (ISystem* system : stage) {
                system->init(); // Assuming ISystem has init()
            }
        }
    }

    void SystemScheduler::execute(float delta_time) {
        if (!m_is_baked) {
            throw std::runtime_error("Scheduler must be baked before execution.");
        }

        // This is now the super-fast, frame-by-frame execution path.
        // No graph logic, just iteration.
        for (const auto& stage : m_execution_stages) {
            // Here is where you would integrate a thread pool for parallelism.
            // For now, we execute sequentially.
            for (ISystem* system : stage) {
                system->update(delta_time);
            }
        }
    }

    void SystemScheduler::shutdown() {
        // Execute system shutdown logic in reverse order of execution.
        for (auto it = m_execution_stages.rbegin(); it != m_execution_stages.rend(); ++it) {
            for (ISystem* system : *it) {
                system->shutdown(); // Assuming ISystem has shutdown()
            }
        }
    }

} // namespace RDE