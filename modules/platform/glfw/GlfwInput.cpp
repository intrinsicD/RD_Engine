// RDE_Project/modules/platform/glfw/GlfwInput.cpp
#include "Core/Input.h"
#include "Core/Application.h"
#include <GLFW/glfw3.h>
namespace RDE {
    bool Input::is_key_pressed(int keycode) {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::is_mouse_button_pressed(int button) {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::get_mouse_position() {
        auto window = static_cast<GLFWwindow *>(Application::get().get_window().get_native_window());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return {(float) xpos, (float) ypos};
    }

    float Input::get_mouse_x() { return get_mouse_position().x; }

    float Input::get_mouse_y() { return get_mouse_position().y; }
}