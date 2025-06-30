#pragma once

#include "AssetDatabase.h"
#include "VulkanDevice.h"

namespace RDE {
    class VulkanTextureUploader {
    public:
        // Processes all CPU textures in the database that don't have a GPU counterpart.
        void process_uploads(AssetDatabase& asset_db, VulkanDevice& device);
    };
}