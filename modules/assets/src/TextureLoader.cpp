#include "AssetManager.h"
#include "TextureAsset.h"
#include "Log.h"

#include <stb_image.h>

namespace RDE {

    template<>
    AssetHandle AssetManager::load<TextureAsset>(const std::filesystem::path &path) {
        // 1. Check registry to avoid duplicate loads.
        if (asset_registry.contains(path)) {
            return asset_registry.at(path);
        }

        // 2. Create the asset struct.
        auto texture = std::make_shared<TextureAsset>();

        // 3. Load from disk.
        stbi_set_flip_vertically_on_load(true); // Common for OpenGL
        texture->pixel_data = stbi_load(path.string().c_str(), &texture->width, &texture->height, &texture->channels, 0);

        if (!texture->pixel_data) {
            RDE_CORE_ERROR("Failed to load texture from path: {}", path.string());
            return INVALID_ASSET_ID;
        }

        // 4. Register the new asset and return its handle.
        AssetHandle handle = NewID();
        asset_registry[path] = handle;
        assets[handle] = texture;

        RDE_CORE_INFO("Texture loaded: {}", path.string());
        return handle;
    }
}
