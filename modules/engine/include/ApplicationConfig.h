#pragma once

namespace RDE::Config {
    struct WindowConfig {
        std::string title = "RD_Engine";
        unsigned int width = 1280;
        unsigned int height = 720;
    };

    enum class RendererAPI {
        OpenGL_4_5 = 0,
        Vulkan_1_2 = 1
    };

    struct RendererConfig {
        RendererAPI api = RendererAPI::OpenGL_4_5;
        bool vsync = true; // Enable or disable vertical sync
    };
}