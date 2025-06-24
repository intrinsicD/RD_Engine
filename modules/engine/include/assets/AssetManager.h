#pragma once

#include "IAsset.h" // You must have this file (see below)
#include "AssetHandle.h" // You must have this file (see below)
#include "IAssetLoader.h" // You must have this file (see below)
#include "utils/FileIOUtils.h" // You must have this file (see below)
#include "Log.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

// It's good practice to put logging/assertions in a central place.
// For this example, we'll just use iostream and assert.
#include <cassert>


// Provide a hash implementation for AssetHandle so it can be used in std::unordered_map
namespace std {
    template<>
    struct hash<RDE::AssetHandle> {
        size_t operator()(const RDE::AssetHandle &handle) const {
            return hash<uint32_t>()(handle.get_asset_id());
        }
    };
}

namespace RDE {
    class AssetManager {
    public:
        AssetManager() = default;

        ~AssetManager() = default;

        void register_asset_loader(const std::shared_ptr<IAssetLoader>& loader) {
            auto ext = loader->get_extension();
            m_extension_to_type_map[ext] = loader->get_asset_type();
            m_asset_loaders[ext] = [loader](const std::string &path) {
                return loader->load(path);
            };
        }

        std::shared_ptr<IAssetLoader> get_loader(const std::string &ext) {
            //TODO currently we store a generic std::function in the map so we cannot return a specific loader.
            return nullptr; // This should be replaced with actual logic to return the specific loader if needed.
        }

        template<typename T>
        T *get_asset(AssetID id) {
            //TODO: Implement this function to retrieve an asset by its ID.
            return nullptr;
        }

        template<typename T>
        T *load_asset(const std::string &path) {
            auto loader = get_loader(FileIO::GetFileExtension(path));
            if(!loader) {
                RDE_CORE_ERROR("No loader registered for asset type: {}", FileIO::GetFileExtension(path));
                return nullptr;
            }
            //TODO: Implement this function to retrieve an asset by its ID.
            return nullptr;
        }

        /**
         * @brief Registers a function that knows how to load a specific asset type from a file path.
         * @tparam T The asset type (e.g., Texture, Mesh) this loader is for.
         * @param loader_func A function (often a lambda) that takes a const std::string& path and returns a std::shared_ptr<T>.
         */
        template<typename T>
        void
        register_loader(const std::string &ext, std::function<std::shared_ptr<T>(const std::string &)> loader_func) {
            // We store a generic version of the loader that returns a shared_ptr to the base IAsset.
            m_asset_loaders[ext] = [loader_func](const std::string &path) {
                return std::dynamic_pointer_cast<IAsset>(loader_func(path));
            };
            RDE_CORE_INFO("Registered loader for asset type: {}", ext);
        }

        /**
         * @brief Loads an asset from a given path, or returns a handle to it if already loaded.
         * @tparam T The type of asset to load. A loader for this type must be registered.
         * @param path The file path of the asset.
         * @return A valid AssetHandle on success, or an invalid handle on failure.
         */
        template<typename T>
        AssetHandle load(const std::string &path) {
            // 1. Check if this path has already been loaded (fast path).
            auto cache_it = m_path_to_handle_cache.find(path);
            if (cache_it != m_path_to_handle_cache.end()) {
                RDE_CORE_INFO("Asset cache hit for path: {}", path);
                return cache_it->second;
            }

            // 2. Find the correct loader for the requested asset type 'T'.
            auto ext = FileIO::GetFileExtension(path);
            auto loader_it = m_asset_loaders.find(ext);
            if (loader_it == m_asset_loaders.end()) {
                RDE_CORE_ERROR("No loader registered for asset type: {}", ext);
                return AssetHandle{INVALID_ASSET_ID};
            }

            // 3. Call the specialized loader function.
            RDE_CORE_INFO("Loading asset of type {} from path: {}", ext, path);
            std::shared_ptr<IAsset> new_asset = loader_it->second(path);

            // 4. Check if the load was successful.
            if (!new_asset) {
                RDE_CORE_ERROR("Failed to load asset from path: {}", path);
                return AssetHandle{INVALID_ASSET_ID};
            }

            // 5. Generate a new handle and store the asset.
            AssetID new_id = m_next_asset_id++;
            AssetHandle new_handle{new_id};

            m_assets[new_handle] = new_asset;
            m_path_to_handle_cache[path] = new_handle;

            RDE_CORE_INFO("Loaded asset with ID: {} from path: {}", new_id, path);
            return new_handle;
        }

        /**
         * @brief Retrieves a pointer to a loaded asset.
         * @tparam T The type of asset to retrieve.
         * @param handle The handle of the asset.
         * @return A pointer to the asset of type T, or nullptr if the handle is invalid or the type is wrong.
         */
        template<typename T>
        T *get(AssetHandle handle) {
            if (!handle.is_valid()) {
                return nullptr;
            }

            auto it = m_assets.find(handle);
            if (it != m_assets.end()) {
                // Use dynamic_cast to safely convert from IAsset* to the requested derived type T*.
                // This will return nullptr if the stored asset is not of type T, preventing type errors.
                return dynamic_cast<T *>(it->second.get());
            }

            RDE_CORE_ERROR("Asset with handle {} not found", handle.get_asset_id());
            return nullptr;
        }

        AssetType get_asset_type(const std::string &path) const {
            auto ext = FileIO::GetFileExtension(path);
            auto it = m_extension_to_type_map.find(ext);
            if (it != m_extension_to_type_map.end()) {
                return it->second;
            }
            return AssetType::None; // Return None if no type is found for the extension
        }

    private:
        // Stores the actual loaded asset data, mapped by their unique handle.
        std::unordered_map<AssetHandle, std::shared_ptr<IAsset>> m_assets;

        // A cache to prevent loading the same file multiple times. Maps path -> handle.
        std::unordered_map<std::string, AssetHandle> m_path_to_handle_cache;

        // A map of type_index -> function for loading a specific asset type.
        std::unordered_map<std::string, std::function<std::shared_ptr<IAsset>(const std::string &)>> m_asset_loaders;

        // Maps file extensions to their corresponding AssetType.
        std::unordered_map<std::string, AssetType> m_extension_to_type_map;

        // A simple counter for generating new, unique AssetIDs.
        AssetID m_next_asset_id = 1; // Start at 1, since 0 is INVALID_ASSET_ID
    };
}