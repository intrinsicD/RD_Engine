// file: modules/assets/include/Assets/TextureAsset.h
#pragma once

#include "IAsset.h"

#include <stb_image.h> // For stbi_image_free

namespace RDE {

    struct TextureAsset : public IAsset {
        // --- DATA POPULATED BY ASSETMANAGER ---
        unsigned char* pixel_data = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;

        // --- DATA POPULATED BY RENDERER ---
        // The ID of the texture object on the GPU.
        uint32_t renderer_id = 0;

        ~TextureAsset() override {
            // The AssetManager uses stb_image to load, so we must use
            // stb_image to free the pixel data to avoid memory corruption.
            if (pixel_data) {
                stbi_image_free(pixel_data);
                pixel_data = nullptr;
            }
        }
    };

} // namespace RDE