#pragma once

#include "ISystem.h"
#include "SystemDependencyBuilder.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <entt/fwd.hpp>

namespace RDE {
    struct SystemHandle {
        int id;
    };

    class SystemGraph {
    public:
        SystemHandle register_system(entt::registry &registry, std::unique_ptr<ISystem> system) {
            SystemHandle handle{m_next_id++};
            m_registered_systems[handle.id] = std::move(system);

            SystemDependencyBuilder builder;
            m_registered_systems[handle.id]->declare_dependencies(builder);
            auto reads = builder.get_reads();
            auto writes = builder.get_writes();
            build_edges_for(handle, reads, writes);
            return handle;
        }

        void build_edges_for(SystemHandle new_system_handle, const std::vector<std::type_index> &reads,
                             const std::vector<std::type_index> &writes) {
            int new_system_id = new_system_handle.id;

            // Rule 1 & 2: Read-After-Write and Write-After-Write
            // The new system must run AFTER any system that writes to a component it reads OR writes.
            for (const auto &component_type: reads) {
                if (m_writers.count(component_type)) {
                    // Find all systems that previously registered a write to this component.
                    for (int predecessor_id: m_writers.at(component_type)) {
                        // Add an edge: predecessor -> new_system
                        m_successors[predecessor_id].push_back(new_system_id);
                    }
                }
            }
            for (const auto &component_type: writes) {
                if (m_writers.count(component_type)) {
                    for (int predecessor_id: m_writers.at(component_type)) {
                        if (predecessor_id != new_system_id) { // Avoid self-dependency
                            m_successors[predecessor_id].push_back(new_system_id);
                        }
                    }
                }
            }

            // Rule 3: Write-After-Read
            // The new system, which writes to a component, must run AFTER any system that reads from it.
            for (const auto &component_type: writes) {
                if (m_readers.count(component_type)) {
                    // Find all systems that previously registered a read of this component.
                    for (int predecessor_id: m_readers.at(component_type)) {
                        // Add an edge: predecessor -> new_system
                        m_successors[predecessor_id].push_back(new_system_id);
                    }
                }
            }

            // --- CRITICAL STEP: Update the reverse lookup maps for the NEXT system ---
            for (const auto &component_type: reads) {
                m_readers[component_type].push_back(new_system_id);
            }
            for (const auto &component_type: writes) {
                m_writers[component_type].push_back(new_system_id);
            }
        }

        std::vector<std::vector<ISystem *>> build_execution_stages() {
            std::vector<std::vector<ISystem *>> stages;
            std::unordered_map<int, int> in_degree;

            // --- 1. Initialize in-degree for all registered systems ---
            for (const auto &pair: m_registered_systems) {
                in_degree[pair.first] = 0;
            }

            // --- 2. Calculate the actual in-degree for each node ---
            // For each predecessor, increment the in-degree of its successors.
            for (const auto &pair: m_successors) {
                for (int successor_id: pair.second) {
                    in_degree[successor_id]++;
                }
            }

            // --- 3. Find the initial set of nodes with an in-degree of 0 ---
            // These are the systems with no dependencies, they can run first.
            std::vector<int> current_stage_nodes;
            for (const auto &pair: in_degree) {
                if (pair.second == 0) {
                    current_stage_nodes.push_back(pair.first);
                }
            }

            int processed_count = 0;

            // --- 4. Process stages until there are no nodes left ---
            while (!current_stage_nodes.empty()) {
                std::vector<ISystem *> stage_systems;
                for (int system_id: current_stage_nodes) {
                    stage_systems.push_back(m_registered_systems.at(system_id).get());
                    processed_count++;
                }
                stages.push_back(stage_systems);

                std::vector<int> next_stage_nodes;
                for (int predecessor_id: current_stage_nodes) {
                    // If this node has successors...
                    if (m_successors.count(predecessor_id)) {
                        // ...decrement the in-degree of each successor.
                        for (int successor_id: m_successors.at(predecessor_id)) {
                            in_degree[successor_id]--;
                            // If a successor's in-degree drops to 0, it's ready for the next stage.
                            if (in_degree[successor_id] == 0) {
                                next_stage_nodes.push_back(successor_id);
                            }
                        }
                    }
                }
                current_stage_nodes = next_stage_nodes;
            }

            // --- 5. Check for cycles ---
            // If we didn't process all systems, it means there was a cycle.
            if (processed_count != m_registered_systems.size()) {
                // You can add more detailed logging here to find which nodes are stuck.
                throw std::runtime_error("SystemGraph has cycles, cannot build execution stages.");
            }

            return stages;
        }

    private:
        std::unordered_map<int, std::unique_ptr<ISystem>> m_registered_systems;

        // The graph: maps a system ID to the list of IDs of systems that must run AFTER it.
        // This represents the edges of our DAG.
        std::unordered_map<int, std::vector<int>> m_successors;

        // The reverse lookup maps: for efficient edge building.
        // Maps a component type to all systems that read from it.
        std::unordered_map<std::type_index, std::vector<int>> m_readers;
        // Maps a component type to all systems that write to it.
        std::unordered_map<std::type_index, std::vector<int>> m_writers;
        int m_next_id = 0;
    };
}