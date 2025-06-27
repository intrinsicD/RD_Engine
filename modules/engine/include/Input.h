#pragma once

#include "glm/vec2.hpp"

namespace RDE {
    class Input {
    public:
        static bool IsKeyPressed(void *native_window, int keycode);

        static bool IsMouseButtonPressed(void *native_window, int button);

        static glm::vec2 GetMousePosition(void *native_window);
    };
}
