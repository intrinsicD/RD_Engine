// RDE_Project/modules/platform/glfw/GlfwInput.cpp
#include "Input.h"

#include <GLFW/glfw3.h>

namespace RDE {
    bool Input::IsKeyPressed(void *native_window, int keycode) {
        auto window = static_cast<GLFWwindow *>(native_window);
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(void *native_window, int button) {
        auto window = static_cast<GLFWwindow *>(native_window);
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition(void *native_window) {
        auto window = static_cast<GLFWwindow *>(native_window);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return {(float) xpos, (float) ypos};
    }
}
