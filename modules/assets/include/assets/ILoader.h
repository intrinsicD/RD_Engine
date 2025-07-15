#pragma once

#include "AssetDatabase.h"

#include <string>
#include <memory>
#include <vector>

namespace RDE{
    class ILoader {
    public:
        virtual ~ILoader() = default;

        // This is the main function that will be called to load an asset.
        // It should return a pointer to the loaded asset or nullptr on failure.
        virtual AssetID load(const std::string &uri, AssetDatabase &asset_database) const = 0;

        // Optionally, you can provide a method to get the supported file extensions.
        virtual std::vector<std::string> get_supported_extensions() const = 0;
    };
}