#pragma once

#include "AssetDatabase.h"

#include <string>
#include <memory>
#include <vector>

namespace RDE{
    class ILoader {
    public:
        virtual ~ILoader() = default;

        // --- NEW: Fast dependency discovery ---
        // Reads the minimal amount of a file to find its dependent asset URIs.
        // For a GLTF, this might mean just parsing the JSON part.
        // For a material file, just reading the texture paths.
        virtual std::vector<std::string> get_dependencies(const std::string& uri) const = 0;

        // --- REVISED: The actual loading function ---
        // This function assumes dependencies are already being handled by the AssetManager.
        // The AssetManager itself is passed in to allow loaders to request nested dependencies.
        virtual AssetID load_asset(const std::string& uri,
                                   AssetDatabase& asset_database,
                                   AssetManager& asset_manager) const = 0;

        // Optionally, you can provide a method to get the supported file extensions.
        virtual std::vector<std::string> get_supported_extensions() const = 0;

        virtual bool check_version(int version) const {
            // Default implementation returns true for version 1
            return version == 1;
        }
    };
}