#pragma once

namespace RDE::Config {


    enum class RendererAPI {
        OpenGL_4_5 = 0,
        Vulkan_1_2 = 1
    };

    struct RendererConfig {
        RendererAPI api = RendererAPI::OpenGL_4_5;
        bool vsync = true; // Enable or disable vertical sync
    };
}