// RDE_Project/modules/platform/glfw/GlfwInput.cpp
#include "Input.h"
#include "Application.h"
#include "IWindow.h"
#include <GLFW/glfw3.h>

namespace RDE {
    bool Input::IsKeyPressed(int keycode) {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(int button) {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition() {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return {(float) xpos, (float) ypos};
    }

    float Input::GetMouseX() { return GetMousePosition().x; }

    float Input::GetMouseY() { return GetMousePosition().y; }
}
