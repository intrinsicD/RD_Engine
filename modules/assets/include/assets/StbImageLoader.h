// assets/loaders/StbImageLoader.h
#pragma once

#include "assets/ILoader.h"
#include "assets/AssetDatabase.h"
#include "assets/AssetComponentTypes.h"
#include "core/Log.h"

#include <stb_image.h>
#include <filesystem>

namespace RDE {
    class StbImageLoader final : public ILoader {
    public:
        // No constructor or members needed. The loader is stateless.
        StbImageLoader() = default;

        // --- 1. Fast Dependency Discovery ---
        // A standard image file has no external file dependencies.
        // Therefore, this function correctly returns an empty list.
        std::vector<std::string> get_dependencies(const std::string& uri) const override {
            // Unused parameter `uri`
            (void)uri;
            return {};
        }

        // --- 2. The Actual Loading Function ---
        // This function's logic is almost identical to your original `load` function.
        // It's just renamed to `load_asset` to match the new interface.
        AssetID load_asset(const std::string& uri, AssetDatabase& db, AssetManager& manager) const override {
            // Unused parameter `manager` as textures have no dependencies to look up.
            (void)manager;

            RDE_CORE_INFO("StbImageLoader: Loading texture from '{}'...", uri);

            int width, height, channels;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* data = stbi_load(uri.c_str(), &width, &height, &channels, 0);

            if (!data) {
                RDE_CORE_ERROR("StbImageLoader: Failed to load texture '{}'. Reason: {}", uri, stbi_failure_reason());
                return nullptr;
            }

            // Create the CPU-side resource component.
            AssetCpuTexture cpu_texture;
            cpu_texture.width = width;
            cpu_texture.height = height;
            cpu_texture.channels = channels;
            size_t image_size = width * height * channels;
            cpu_texture.data.assign(data, data + image_size);

            // Free the temporary buffer allocated by stb_image.
            stbi_image_free(data);

            // Populate the AssetDatabase with the new entity and its components.
            auto& registry = db.get_registry();
            auto entity_id = registry.create();
            
            registry.emplace<AssetCpuTexture>(entity_id, std::move(cpu_texture));
            registry.emplace<AssetFilepath>(entity_id, uri);
            registry.emplace<AssetName>(entity_id, std::filesystem::path(uri).filename().string());

            RDE_CORE_TRACE("StbImageLoader: Successfully populated asset for '{}'", uri);
            return std::make_shared<AssetID_Data>(entity_id, uri);
        }

        // --- 3. Unchanged ---
        std::vector<std::string> get_supported_extensions() const override {
            return {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".hdr"};
        }
    };
} // namespace RDE```