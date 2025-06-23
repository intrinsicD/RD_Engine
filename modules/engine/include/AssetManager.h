#pragma once

#include "IAsset.h" // You must have this file (see below)

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

// It's good practice to put logging/assertions in a central place.
// For this example, we'll just use iostream and assert.
#include <iostream>
#include <cassert>

namespace RDE {

    // --- AssetHandle ---
    // Represents a type-safe, lightweight reference to an asset.
    using AssetID = uint32_t;
    const AssetID INVALID_ASSET_ID = 0;

    class AssetHandle {
    public:
        AssetHandle(AssetID id = INVALID_ASSET_ID) : m_asset_id(id) {}

        AssetID get_asset_id() const {
            return m_asset_id;
        }

        bool is_valid() const {
            return m_asset_id != INVALID_ASSET_ID;
        }

        // Overloads for use as a key in maps/sets
        bool operator==(const AssetHandle& other) const {
            return m_asset_id == other.m_asset_id;
        }
        bool operator!=(const AssetHandle& other) const {
            return m_asset_id != other.m_asset_id;
        }

    private:
        AssetID m_asset_id;
    };
}

// Provide a hash implementation for AssetHandle so it can be used in std::unordered_map
namespace std {
    template <>
    struct hash<RDE::AssetHandle> {
        size_t operator()(const RDE::AssetHandle& handle) const {
            return hash<uint32_t>()(handle.get_asset_id());
        }
    };
}

namespace RDE {
    class AssetManager {
    public:
        AssetManager() = default;

        ~AssetManager() = default;

        /**
         * @brief Registers a function that knows how to load a specific asset type from a file path.
         * @tparam T The asset type (e.g., Texture, Mesh) this loader is for.
         * @param loader_func A function (often a lambda) that takes a const std::string& path and returns a std::shared_ptr<T>.
         */
        template<typename T>
        void register_loader(std::function<std::shared_ptr<T>(const std::string&)> loader_func) {
            // We store a generic version of the loader that returns a shared_ptr to the base IAsset.
            m_asset_loaders[typeid(T)] = [loader_func](const std::string& path) {
                return std::dynamic_pointer_cast<IAsset>(loader_func(path));
            };
            std::cout << "Registered loader for type: " << typeid(T).name() << std::endl;
        }

        /**
         * @brief Loads an asset from a given path, or returns a handle to it if already loaded.
         * @tparam T The type of asset to load. A loader for this type must be registered.
         * @param path The file path of the asset.
         * @return A valid AssetHandle on success, or an invalid handle on failure.
         */
        template<typename T>
        AssetHandle load(const std::string& path) {
            // 1. Check if this path has already been loaded (fast path).
            auto cache_it = m_path_to_handle_cache.find(path);
            if (cache_it != m_path_to_handle_cache.end()) {
                std::cout << "Asset cache hit for path: " << path << std::endl;
                return cache_it->second;
            }

            // 2. Find the correct loader for the requested asset type 'T'.
            auto loader_it = m_asset_loaders.find(typeid(T));
            if (loader_it == m_asset_loaders.end()) {
                std::cerr << "Error: No loader registered for asset type " << typeid(T).name() << std::endl;
                return AssetHandle{ INVALID_ASSET_ID };
            }

            // 3. Call the specialized loader function.
            std::cout << "Loading asset of type " << typeid(T).name() << " from path: " << path << std::endl;
            std::shared_ptr<IAsset> new_asset = loader_it->second(path);

            // 4. Check if the load was successful.
            if (!new_asset) {
                std::cerr << "Error: Loader failed to load asset from path: " << path << std::endl;
                return AssetHandle{ INVALID_ASSET_ID };
            }

            // 5. Generate a new handle and store the asset.
            AssetID new_id = m_next_asset_id++;
            AssetHandle new_handle{ new_id };

            m_assets[new_handle] = new_asset;
            m_path_to_handle_cache[path] = new_handle;

            std::cout << "Successfully loaded asset. Assigned ID: " << new_id << std::endl;
            return new_handle;
        }

        /**
         * @brief Retrieves a pointer to a loaded asset.
         * @tparam T The type of asset to retrieve.
         * @param handle The handle of the asset.
         * @return A pointer to the asset of type T, or nullptr if the handle is invalid or the type is wrong.
         */
        template<typename T>
        T* get(AssetHandle handle) {
            if (!handle.is_valid()) {
                return nullptr;
            }

            auto it = m_assets.find(handle);
            if (it != m_assets.end()) {
                // Use dynamic_cast to safely convert from IAsset* to the requested derived type T*.
                // This will return nullptr if the stored asset is not of type T, preventing type errors.
                return dynamic_cast<T*>(it->second.get());
            }

            std::cerr << "Warning: Attempted to get asset with invalid handle: " << handle.get_asset_id() << std::endl;
            return nullptr;
        }

    private:
        // Stores the actual loaded asset data, mapped by their unique handle.
        std::unordered_map<AssetHandle, std::shared_ptr<IAsset>> m_assets;

        // A cache to prevent loading the same file multiple times. Maps path -> handle.
        std::unordered_map<std::string, AssetHandle> m_path_to_handle_cache;

        // A map of type_index -> function for loading a specific asset type.
        std::unordered_map<std::type_index, std::function<std::shared_ptr<IAsset>(const std::string&)>> m_asset_loaders;

        // A simple counter for generating new, unique AssetIDs.
        AssetID m_next_asset_id = 1; // Start at 1, since 0 is INVALID_ASSET_ID
    };
}