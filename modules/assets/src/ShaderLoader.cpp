// In modules/core/src/Assets/AssetManager.cpp
#include "ShaderAsset.h"
#include "AssetManager.h"
#include "Log.h"
#include "FileIO.h"

namespace RDE {
    // --- Shader Specialization ---
    template<>
    AssetHandle AssetManager::load<ShaderAsset>(const std::filesystem::path& path) {
        // We will adopt a naming convention: my_shader.vert, my_shader.frag
        if (asset_registry.contains(path)) {
            return asset_registry.at(path);
        }

        auto shader = std::make_shared<ShaderAsset>();
        bool success = true;

        // 1. Load Vertex Shader
        shader->vertex_source = FileIO::read_file(path);
        if (shader->vertex_source.empty()) {
            RDE_CORE_ERROR("Failed to read vertex shader source: {}", path.string());
            success = false;
        }

        // 2. Load Fragment Shader (by convention)
        std::filesystem::path fragment_path = path;
        fragment_path.replace_extension(".frag");
        shader->fragment_source = FileIO::read_file(fragment_path);
        if (shader->fragment_source.empty()) {
            RDE_CORE_WARN("Could not find or read fragment shader source: {}", fragment_path.string());
            // This might not be an error if it's a compute-only shader, but for now we warn.
        }

        // NOTE: Add similar logic for geometry, compute shaders if the paths exist.

        if (!success) {
            return INVALID_ASSET_ID;
        }

        // 3. Register the asset.
        AssetHandle handle = NewID();
        asset_registry[path] = handle;
        assets[handle] = shader;

        RDE_CORE_INFO("Shader source loaded: {}", path.string());
        return handle;
    }
}