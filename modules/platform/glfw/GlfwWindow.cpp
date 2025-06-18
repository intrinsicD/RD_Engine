// RDE_Project/modules/platform/glfw/GlfwWindow.cpp

#include <glad/gl.h>
#include "GlfwWindow.h"
#include "Core/Log.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/KeyEvent.h"

namespace RDE {
    static bool s_glfw_initialized = false;

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    std::unique_ptr<Window> Window::Create(const WindowProps &props) {
        return std::make_unique<GlfwWindow>(props);
    }

    GlfwWindow::GlfwWindow(const WindowProps &props) {
        init(props);
    }

    GlfwWindow::~GlfwWindow() {
        shutdown();
    }

    void GlfwWindow::init(const WindowProps &props) {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        RDE_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

        if (!s_glfw_initialized) {
            int success = glfwInit();
            RDE_CORE_ASSERT(success, "Could not initialize GLFW!");
            glfwSetErrorCallback(GlfwErrorCallback);
            s_glfw_initialized = true;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow((int) props.width, (int) props.height, m_data.title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);
        int status = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
        RDE_CORE_ASSERT(status, "Failed to initialize Glad!");

        glfwSetWindowUserPointer(m_window, &m_data);
        set_vsync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            WindowResizeEvent event(width, height);
            data.event_callback(event);
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.event_callback(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data.event_callback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.event_callback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    data.event_callback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data.event_callback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data.event_callback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double x_offset, double y_offset) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float) x_offset, (float) y_offset);
            data.event_callback(event);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x_pos, double y_pos) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float) x_pos, (float) y_pos);
            data.event_callback(event);
        });
    }

    void GlfwWindow::shutdown() {
        glfwDestroyWindow(m_window);
    }

    void GlfwWindow::on_update() {
        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    void GlfwWindow::set_vsync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        m_data.vsync = enabled;
    }

    bool GlfwWindow::is_vsync() const {
        return m_data.vsync;
    }
}