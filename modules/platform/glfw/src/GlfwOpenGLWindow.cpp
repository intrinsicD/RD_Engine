// RDE_Project/modules/platform/glfw/GlfwWindow.cpp

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "GlfwOpenGLWindow.h"
#include "Log.h"
#include "events/ApplicationEvent.h"
#include "events/MouseEvent.h"
#include "events/KeyEvent.h"

namespace RDE {
    static bool s_glfw_initialized = false;

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    std::shared_ptr<IWindow> IWindow::Create(const WindowConfig &window_config) {
        return std::make_shared<GlfwOpenGLWindow>(window_config);
    }

    GlfwOpenGLWindow::GlfwOpenGLWindow(const WindowConfig &window_config) : m_window(nullptr) {
        m_data.title = window_config.title;
        m_data.width = window_config.width;
        m_data.height = window_config.height;
    }

    GlfwOpenGLWindow::~GlfwOpenGLWindow() {
        shutdown();
    }

    bool GlfwOpenGLWindow::init() {
        RDE_CORE_INFO("Creating window {} ({}, {})", m_data.title, m_data.width, m_data.height);

        if (!s_glfw_initialized) {
            int success = glfwInit();
            RDE_CORE_ASSERT(success, "Could not initialize GLFW!");
            if (!success) return false;

            glfwSetErrorCallback(GlfwErrorCallback);
            s_glfw_initialized = true;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow((int) m_data.width, (int) m_data.height, m_data.title.c_str(), nullptr, nullptr);

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
            if (data.event_callback) {
                data.event_callback(event);
            }
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            if (data.event_callback) {
                data.event_callback(event);
            }
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double x_offset, double y_offset) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float) x_offset, (float) y_offset);
            if (data.event_callback) {
                data.event_callback(event);
            }
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x_pos, double y_pos) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float) x_pos, (float) y_pos);
            if (data.event_callback) {
                data.event_callback(event);
            }
        });

        glfwSetDropCallback(m_window, [](GLFWwindow *window, int count, const char **filepaths) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            std::vector<std::string> files;
            files.reserve(count);
            for (int i = 0; i < count; ++i) {
                files.emplace_back(filepaths[i]);
            }
            WindowFileDropEvent event(files);
            if (data.event_callback) {
                data.event_callback(event);
            }
        });
        return true;
    }

    void GlfwOpenGLWindow::shutdown() {
        RDE_CORE_INFO("Shutting down window {}", m_data.title);
        glfwDestroyWindow(m_window);
    }

    void GlfwOpenGLWindow::poll_events() {
        glfwPollEvents();
    }

    void GlfwOpenGLWindow::on_update() {

    }

    void GlfwOpenGLWindow::set_vsync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        m_data.vsync = enabled;
    }

    bool GlfwOpenGLWindow::is_vsync() const {
        return m_data.vsync;
    }

    void GlfwOpenGLWindow::close() {
        glfwSetWindowShouldClose(m_window, true);
    }

    void GlfwOpenGLWindow::swap_buffers() {
        glfwSwapBuffers(m_window);
    }
}
