// file: Core/DependencyGraph.h
#pragma once

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <functional>

namespace RDE {
    // A generic handle to a node in the graph.
    // Light and cheap to copy.
    template<typename T>
    struct GraphNodeHandle {
        size_t id;
        bool operator==(const GraphNodeHandle &other) const { return id == other.id; }
    };
}

// Hash specialization to allow GraphNodeHandle to be used in unordered_maps.
namespace std {
    template<typename T>
    struct hash<RDE::GraphNodeHandle<T> > {
        size_t operator()(const RDE::GraphNodeHandle<T> &handle) const {
            return hash<size_t>()(handle.id);
        }
    };
}

namespace RDE {
    template<typename TNodePayload, typename TResourceHandle>
    class DependencyGraph {
    public:
        using NodeHandle = GraphNodeHandle<TNodePayload>;

        NodeHandle add_node(TNodePayload payload,
                            std::vector<TResourceHandle> reads,
                            std::vector<TResourceHandle> writes) {
            NodeHandle handle{m_nodes.size()};
            m_nodes.emplace_back(InternalNode{std::move(payload)});

            // --- This is the core dependency-building logic, now generic ---

            // RAW/WAW: This new node must run AFTER any node that writes to its reads/writes.
            for (const auto &resource: reads) {
                if (m_resource_writers.count(resource)) {
                    for (const auto &writer_handle: m_resource_writers.at(resource)) {
                        add_edge(writer_handle, handle);
                    }
                }
            }
            for (const auto &resource: writes) {
                if (m_resource_writers.count(resource)) {
                    for (const auto &writer_handle: m_resource_writers.at(resource)) {
                        add_edge(writer_handle, handle);
                    }
                }
            }

            // WAR: This new node must run AFTER any node that reads from its writes.
            for (const auto &resource: writes) {
                if (m_resource_readers.count(resource)) {
                    for (const auto &reader_handle: m_resource_readers.at(resource)) {
                        add_edge(reader_handle, handle);
                    }
                }
            }

            // Update the lookup tables for future nodes.
            for (const auto &resource: reads) {
                m_resource_readers[resource].push_back(handle);
            }
            for (const auto &resource: writes) {
                m_resource_writers[resource].push_back(handle);
            }

            return handle;
        }

        // "Bakes" the graph into executable stages, performing a topological sort.
        std::vector<std::vector<const TNodePayload *> > bake() {
            std::vector<std::vector<const TNodePayload *> > stages;
            std::unordered_map<NodeHandle, int> in_degree;
            std::vector<NodeHandle> initial_nodes;

            // Initialize in-degrees
            for (size_t i = 0; i < m_nodes.size(); ++i) {
                NodeHandle handle{i};
                in_degree[handle] = 0;
            }

            for (const auto &node: m_nodes) {
                for (const auto &successor: node.successors) {
                    in_degree[successor]++;
                }
            }

            // Find initial nodes (in-degree == 0)
            for (const auto &pair: in_degree) {
                if (pair.second == 0) {
                    initial_nodes.push_back(pair.first);
                }
            }

            size_t processed_count = 0;
            std::vector<NodeHandle> current_stage_nodes = initial_nodes;

            while (!current_stage_nodes.empty()) {
                std::vector<const TNodePayload *> stage_payloads;
                for (const auto &handle: current_stage_nodes) {
                    stage_payloads.push_back(&m_nodes[handle.id].payload);
                    processed_count++;
                }
                stages.push_back(stage_payloads);

                std::vector<NodeHandle> next_stage_nodes;
                for (const auto &handle: current_stage_nodes) {
                    for (const auto &successor_handle: m_nodes[handle.id].successors) {
                        in_degree[successor_handle]--;
                        if (in_degree[successor_handle] == 0) {
                            next_stage_nodes.push_back(successor_handle);
                        }
                    }
                }
                current_stage_nodes = std::move(next_stage_nodes);
            }

            if (processed_count != m_nodes.size()) {
                throw std::runtime_error("DependencyGraph has a cycle!");
            }

            return stages;
        }

        void clear() {
            m_nodes.clear();
            m_resource_readers.clear();
            m_resource_writers.clear();
        }

    private:
        struct InternalNode {
            TNodePayload payload;
            std::vector<NodeHandle> successors; // Nodes that depend on this one
        };

        void add_edge(NodeHandle predecessor, NodeHandle successor) {
            // Avoid self-loops and duplicate edges
            if (predecessor.id == successor.id) return;
            for (const auto &existing_succ: m_nodes[predecessor.id].successors) {
                if (existing_succ.id == successor.id) return;
            }
            m_nodes[predecessor.id].successors.push_back(successor);
        }

        std::vector<InternalNode> m_nodes;
        std::unordered_map<TResourceHandle, std::vector<NodeHandle> > m_resource_readers;
        std::unordered_map<TResourceHandle, std::vector<NodeHandle> > m_resource_writers;
    };
}
