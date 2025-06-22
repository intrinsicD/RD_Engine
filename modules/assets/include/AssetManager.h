// file: modules/assets/include/AssetManager.h
#pragma once

#include "IAsset.h"
#include "AssetHandle.h"

#include <filesystem>
#include <unordered_map>
#include <memory>

namespace RDE {
    struct ShaderAsset;
    struct TextureAsset;
    struct MaterialAsset;
    struct MeshAsset;

    class AssetManager {
    public:
        AssetManager() = default;

        ~AssetManager() = default;

        // Non-copyable, but movable
        AssetManager(const AssetManager &) = delete;

        AssetManager &operator=(const AssetManager &) = delete;

        AssetManager(AssetManager &&) = default;

        AssetManager &operator=(AssetManager &&) = default;

        template<typename T>
        AssetHandle load(const std::filesystem::path &path);

        template<typename T>
        T *get(AssetHandle handle);

    private:
        static AssetID NewID() { return ++next_id; }
        // Stores the actual asset data, type-erased via IAsset.
        std::unordered_map<AssetHandle, std::shared_ptr<IAsset> > assets{};

        // Maps a file path to a handle for quick lookup to prevent duplicate loading.
        std::unordered_map<std::filesystem::path, AssetHandle> asset_registry{};

        static AssetID next_id;
    };

    template<typename T>
    AssetHandle AssetManager::load(const std::filesystem::path &path) {
        static_assert(sizeof(T) == 0,
                      "AssetManager::Load<T> not specialized for this type. Please create a specialization.");
        return INVALID_ASSET_ID;
    }

    template<>
    AssetHandle AssetManager::load<ShaderAsset>(const std::filesystem::path &path);

    template<>
    AssetHandle AssetManager::load<MeshAsset>(const std::filesystem::path &path);

    template<>
    AssetHandle AssetManager::load<TextureAsset>(const std::filesystem::path &path);

    template<>
    AssetHandle AssetManager::load<MaterialAsset>(const std::filesystem::path &path);

    template<typename T>
    T *AssetManager::get(AssetHandle handle) {
        if (handle == INVALID_ASSET_ID) {
            return nullptr;
        }

        auto it = assets.find(handle);
        if (it == assets.end()) {
            return nullptr;
        }

        std::shared_ptr<T> typed_ptr = std::dynamic_pointer_cast<T>(it->second);

        if (!typed_ptr) {
            return nullptr;
        }

        return typed_ptr.get();
    }
} // namespace RDE
