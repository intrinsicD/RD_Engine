// file: Core/ContextComponents/WindowContextComponent.h
#pragma once

#include <cstdint>

namespace RDE {

    struct WindowContextComponent {
        void* native_window_handle = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        float aspect_ratio = 1.0f;
        bool vsync_enabled = false;
        bool is_focused = false;
    };

} // namespace RDE