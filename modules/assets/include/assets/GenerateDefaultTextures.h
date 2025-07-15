// In some setup file, e.g., SandboxApp::init() or a separate tool
#pragma once

#include "core/Log.h"
#include "core/Paths.h"

#include <vector>
#include <string>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace RDE {
    inline void GenerateDefaultTextures() {
        auto textures_path = get_asset_path().value() /  "textures";
        if (!std::filesystem::exists(textures_path)) {
            std::filesystem::create_directories(textures_path);
        }

        int width = 1;
        int height = 1;
        int channels = 4; // RGBA

        // 1. Generate default_white.png
        std::vector<uint8_t> white_pixel = {255, 255, 255, 255};
        std::string white_path = textures_path / "default_white.png";
        if (!std::filesystem::exists(white_path)) {
            stbi_write_png(white_path.c_str(), width, height, channels, white_pixel.data(), width * channels);
            RDE_CORE_INFO("Generated default texture: {}", white_path);
        }

        // 2. Generate default_normal.png
        std::vector<uint8_t> normal_pixel = {128, 128, 255, 255};
        std::string normal_path = textures_path  / "default_normal.png";
        if (!std::filesystem::exists(normal_path)) {
            stbi_write_png(normal_path.c_str(), width, height, channels, normal_pixel.data(), width * channels);
            RDE_CORE_INFO("Generated default texture: {}", normal_path);
        }

        // 3. Generate default_metal_rough.png
        // R=Occlusion, G=Roughness, B=Metalness
        std::vector<uint8_t> pbr_pixel = {255, 128, 0, 255};
        std::string pbr_path = textures_path / "default_metal_rough.png";
        if (!std::filesystem::exists(pbr_path)) {
            stbi_write_png(pbr_path.c_str(), width, height, channels, pbr_pixel.data(), width * channels);
            RDE_CORE_INFO("Generated default texture: {}", pbr_path);
        }
    }
}