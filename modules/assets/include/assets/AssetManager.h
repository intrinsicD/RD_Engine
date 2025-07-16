// assets/AssetManager.h
#pragma once

#include "AssetHandle.h"
#include "AssetDatabase.h"
#include "ILoader.h"
#include "core/DependencyGraph.h"
#include "core/Log.h"

#include <future>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <queue>

namespace RDE {
    class AssetManager {
    public:
        explicit AssetManager(AssetDatabase &asset_database) : m_database(asset_database) {}

        ~AssetManager() = default;

        void register_loader(const std::shared_ptr<ILoader> &loader) {
            for (const auto &ext: loader->get_supported_extensions()) {
                m_loaders[ext] = loader;
            }
        }

        // --- PRIMARY NEW FUNCTION: Asynchronous Loading ---
        std::future<AssetID> load_async(const std::string& uri) {
            // 1. Check cache for already loaded asset.
            if (auto it = m_cache.find(uri); it != m_cache.end()) {
                std::promise<AssetID> promise;
                promise.set_value(it->second);
                RDE_CORE_TRACE("Asset Cache HIT for '{}'.", uri);
                return promise.get_future();
            }

            // 2. Check if this asset is already in the process of being loaded.
            if (auto it = m_loading_operations.find(uri); it != m_loading_operations.end()) {
                RDE_CORE_TRACE("Asset '{}' is already being loaded. Returning existing future.", uri);
                return it->second.get_future();
            }

            // 3. Begin a new loading operation.
            RDE_CORE_INFO("Asset Cache MISS for '{}'. Starting new load operation.", uri);
            m_loading_operations.emplace(uri, std::promise<AssetID>());

            // For simplicity, we do this on the calling thread.
            // In a real engine, you'd dispatch this entire block to a background thread pool.
            try {
                // This is the core logic.
                AssetID result_id = begin_load_operation(uri);
                m_loading_operations.at(uri).set_value(result_id);
            } catch (const std::exception& e) {
                RDE_CORE_ERROR("Failed to load asset '{}': {}", uri, e.what());
                m_loading_operations.at(uri).set_exception(std::current_exception());
            }

            auto future = m_loading_operations.at(uri).get_future();
            // Once the operation is complete, we can remove it from the map.
            // The future remains valid.
            m_loading_operations.erase(uri);
            return future;
        }

        std::future<AssetID> force_load(const std::string &uri) {
            RDE_CORE_INFO("Force loading asset from '{}'.", uri);
            // Clear the cache for this URI to force reload.
            m_cache.erase(uri);

            // Call the regular load function to reload the asset.
            return load_async(uri);
        }

        AssetID get_loaded_asset(const std::string &uri) const {
            auto it = m_cache.find(uri);
            if (it != m_cache.end()) {
                return it->second;
            }
            return nullptr; // Asset not found
        }

        AssetDatabase &get_database() {
            return m_database;
        }
    private:
        AssetID begin_load_operation(const std::string& root_uri) {
            // -- I. DISCOVERY PHASE --
            DependencyGraph<std::string, std::string> graph;
            build_dependency_graph(root_uri, graph);

            // -- II. SCHEDULING PHASE --
            auto stages = graph.bake();

            // -- III. EXECUTION PHASE --
            for (const auto& stage : stages) {
                // All assets in this stage can be loaded in parallel.
                // For now, we'll do it serially to keep it simple.
                // TODO: Replace with parallel_for or dispatch to a thread pool.
                for (const std::string* uri_ptr : stage) {
                    const std::string& current_uri = *uri_ptr;

                    // Skip if it was loaded as a dependency of another parallel asset.
                    if (m_cache.count(current_uri)) continue;

                    std::filesystem::path path(current_uri);
                    std::string ext = path.extension().string();
                    auto it_loader = m_loaders.find(ext);
                    if (it_loader == m_loaders.end()) {
                        throw std::runtime_error("No loader for extension: " + ext);
                    }

                    // The loader does the actual work.
                    AssetID new_id = it_loader->second->load_asset(current_uri, m_database, *this);
                    if (!new_id) {
                        throw std::runtime_error("Loader failed for URI: " + current_uri);
                    }

                    // Cache the result immediately.
                    m_cache[current_uri] = new_id;
                }
            }
            return m_cache.at(root_uri);
        }

        void build_dependency_graph(const std::string& root_uri, DependencyGraph<std::string, std::string>& graph) {
            std::queue<std::string> to_process;
            std::unordered_set<std::string> discovered;

            to_process.push(root_uri);
            discovered.insert(root_uri);

            while (!to_process.empty()) {
                std::string current_uri = to_process.front();
                to_process.pop();

                std::filesystem::path path(current_uri);
                std::string ext = path.extension().string();
                auto it_loader = m_loaders.find(ext);
                if (it_loader == m_loaders.end()) {
                    RDE_CORE_WARN("No loader found for dependency '{}', skipping.", current_uri);
                    continue;
                }

                // Use the new fast discovery method.
                std::vector<std::string> dependencies = it_loader->second->get_dependencies(current_uri);

                // An asset "reads" from its dependencies and "writes" to itself.
                // The payload and resource handle are both the URI string.
                graph.add_node(current_uri, dependencies, {current_uri});

                for (const auto& dep_uri : dependencies) {
                    if (discovered.find(dep_uri) == discovered.end()) {
                        discovered.insert(dep_uri);
                        to_process.push(dep_uri);
                    }
                }
            }
        }

        AssetDatabase &m_database;

        // Your runtime-pluggable system.
        std::unordered_map<std::string, AssetID> m_cache;
        std::unordered_map<std::string, std::shared_ptr<ILoader>> m_loaders;
        std::unordered_map<std::string, std::promise<AssetID>> m_loading_operations;
    };
}
