#pragma once

#include "glm/vec2.hpp"

namespace RDE {
    class Input {
    public:
        static bool IsKeyPressed(int keycode);

        static bool IsMouseButtonPressed(int button);

        static glm::vec2 GetMousePosition();

        static float GetMouseX();

        static float GetMouseY();
    };
}
