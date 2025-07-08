// RDE_Project/modules/platform/glfw/GlfwWindow.cpp


#include "GlfwOpenGLWindow.h"
#include "core/Log.h"
#include "core/events/ApplicationEvent.h"
#include "core/events/MouseEvent.h"
#include "core/events/KeyEvent.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace RDE {
    static void InitializeGLFW() {
        static bool is_initialized = false;
        if (!is_initialized) {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW!");
            }
            // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            is_initialized = true;
        }
    }

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    std::unique_ptr<IWindow> IWindow::Create(const WindowConfig &config) {
        return std::make_unique<GlfwOpenGLWindow>(config);
    }

    GlfwOpenGLWindow::GlfwOpenGLWindow(const WindowConfig &window_config) : m_window(nullptr) {
        m_data.title = window_config.title;
        m_data.width = window_config.width;
        m_data.height = window_config.height;

        InitializeGLFW();
        glfwSetErrorCallback(GlfwErrorCallback);

        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window!");
        }

        glfwMakeContextCurrent(m_window);

        glfwSetWindowUserPointer(m_window, this); // Set the user pointer to this Application instance

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

        int status = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
        RDE_CORE_ASSERT(status, "Failed to initialize Glad!");
    }

    GlfwOpenGLWindow::~GlfwOpenGLWindow() {
        shutdown();
    }

    void GlfwOpenGLWindow::poll_events() {
        glfwPollEvents();
    }

    void GlfwOpenGLWindow::set_vsync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        m_data.vsync = enabled;
    }

    bool GlfwOpenGLWindow::should_close() {
        return glfwWindowShouldClose(m_window);
    }

    void GlfwOpenGLWindow::shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void GlfwOpenGLWindow::swap_buffers() {
        glfwSwapBuffers(m_window);
    }
}
