// RDE/Core/SystemScheduler.h (Revised)
#pragma once

#include "SystemDependencyBuilder.h"
#include "core/DependencyGraph.h"
#include "core/ISystem.h"

#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <typeindex>

namespace RDE {
    class SystemScheduler {
    public:
        SystemScheduler() = default;

        template<typename T, typename... Args>
        void register_system(Args &&... args) {
            // ... (check if baked) ...
            auto system_ptr = std::make_unique<T>(std::forward<Args>(args)...);
            system_ptr->init();

            SystemDependencyBuilder builder;
            system_ptr->declare_dependencies(builder);

            // Get the index this system will have *before* adding it.
            size_t system_index = m_systems.size();

            // The scheduler takes ownership of the system pointer.
            m_systems.push_back(std::move(system_ptr));

            // We use the generic graph. The payload is the system, the resource is the type.
            m_graph.add_node(system_index, builder.get_reads(), builder.get_writes());
            m_is_dirty = true;
        }

        void execute(float delta_time) {
            if (m_is_dirty) {
                bake_internal();
            }

            for (const auto &stage: m_execution_stages) {
                // All systems in 'stage' can run in parallel!
                // For now, we run them sequentially.
                for (const ISystem *system: stage) {
                    // We stored a non-const pointer, so we must const_cast.
                    // A safer design might store a wrapper or index instead of a raw ptr.
                    const_cast<ISystem *>(system)->update(delta_time);
                }
            }
        }

        void shutdown() {
            // Execute system shutdown logic in reverse order of execution.
            for (auto it = m_execution_stages.rbegin(); it != m_execution_stages.rend(); ++it) {
                for (ISystem* system : *it) {
                    system->shutdown(); // Assuming ISystem has shutdown()
                }
            }

            m_execution_stages.clear();
            m_systems.clear();
            m_graph.clear();
        }

    private:

        void bake_internal() {
            // This returns std::vector<std::vector<const size_t*>>
            auto baked_stages_of_indices = m_graph.bake();

            m_execution_stages.clear();
            m_execution_stages.resize(baked_stages_of_indices.size());

            for (size_t i = 0; i < baked_stages_of_indices.size(); ++i) {
                m_execution_stages[i].reserve(baked_stages_of_indices[i].size());
                for (const size_t* index_ptr : baked_stages_of_indices[i]) {
                    // Use the index to get the system pointer from our owned vector.
                    m_execution_stages[i].push_back(m_systems[*index_ptr].get());
                }
            }
            m_is_dirty = false;
        }

        bool m_is_dirty = true;

        DependencyGraph<size_t, std::type_index> m_graph;
        std::vector<std::unique_ptr<ISystem> > m_systems;
        std::vector<std::vector<ISystem *> > m_execution_stages;
    };
} // namespace RDE
