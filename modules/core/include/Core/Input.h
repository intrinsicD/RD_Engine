// RDE_Project/modules/core/include/Core/Input.h
#pragma once

#include <glm/glm.hpp>

class Input {
public:
    static bool IsKeyPressed(int keycode);

    static bool IsMouseButtonPressed(int button);

    static glm::vec2 GetMousePosition();

    static float GetMouseX();

    static float GetMouseY();
};