#pragma once

#include "AssetDatabase.h"          // The database we will populate
#include "AssetComponentTypes.h"
#include "../internal/AssetCreationReceipt.h"
#include "Log.h"
#include <entt/resource/loader.hpp>        // The entt::resource_loader base

// Include stb_image. It's a single header library.
// The IMPLEMENTATION define will go in a .cpp file.
#include <stb_image.h>

#include <filesystem> // For parsing the filename

namespace RDE {

    // A concrete loader for AssetCpuTexture using the stb_image library.
    // It inherits from the entt helper to satisfy the loader concept.
    // The second template argument is the type of resource it returns.
    struct StbImageLoader : entt::resource_loader<AssetCreationReceipt> {
        // This is the main function that entt::resource_cache will call.
        // The arguments are forwarded directly from our AssetManager::load call.
        std::shared_ptr<AssetCreationReceipt> operator()(const std::string& uri, AssetDatabase& db) const {
            RDE_CORE_TRACE("StbImageLoader: Loading texture from '{}'...", uri);

            // 1. --- Perform the actual File I/O and Parsing ---
            int width, height, channels;
            
            // stb_image loads images from the top-left corner, but GPUs often expect the
            // 0.0 V-coordinate at the bottom. Flipping it on load is standard practice.
            stbi_set_flip_vertically_on_load(true);

            // This call reads the file and allocates memory for the image data.
            stbi_uc* data = stbi_load(uri.c_str(), &width, &height, &channels, 0);

            // CRITICAL: Always handle errors.
            if (!data) {
                RDE_CORE_ERROR("StbImageLoader: Failed to load texture '{}'. Reason: {}", uri, stbi_failure_reason());
                return nullptr; // Returning nullptr signals failure to the AssetManager.
            }

            // 2. --- Create the CachedResource and transfer data into it ---

            AssetCpuTexture resource;

            resource.width = width;
            resource.height = height;
            resource.channels = channels;
            
            // Calculate the size and copy the raw pixel data into our managed vector.
            size_t image_size = width * height * channels;
            resource.data.assign(data, data + image_size);

            // CRITICAL: Now that we've copied the data, free the memory allocated by stb_image.
            stbi_image_free(data);

            // 3. --- Populate the AssetDatabase with the new entity and components ---
            auto &registry = db.get_registry();
            auto entity_id = registry.create();
            
            // Emplace the primary data component
            registry.emplace<AssetCpuTexture>(entity_id, resource);

            // Emplace metadata components
            registry.emplace<AssetFilepath>(entity_id, uri);
            
            // Let's also give it a name based on the filename for easier debugging/UI.
            std::string name = std::filesystem::path(uri).filename().string();
            registry.emplace<AssetName>(entity_id, name);

            RDE_CORE_INFO("StbImageLoader: Successfully loaded '{}' ({}x{}, {} channels).", name, width, height, channels);

            // 4. --- Return the completed struct ---
            // The entt::resource_cache will now store this shared_ptr.
            return std::make_shared<AssetCreationReceipt>(entity_id);
        }
    };

} // namespace RDE