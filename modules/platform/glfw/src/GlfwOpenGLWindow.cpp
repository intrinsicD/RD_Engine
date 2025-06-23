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
        RDE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    std::unique_ptr<IWindow> IWindow::Create(const Config::WindowConfig &window_config) {
        return std::make_unique<GlfwOpenGLWindow>(window_config);
    }

    GlfwOpenGLWindow::GlfwOpenGLWindow(const Config::WindowConfig &window_config) {
        init(window_config);
    }

    GlfwOpenGLWindow::~GlfwOpenGLWindow() {
        shutdown();
    }

    void GlfwOpenGLWindow::init(const Config::WindowConfig &window_config) {
        m_data.title = window_config.title;
        m_data.width = window_config.width;
        m_data.height = window_config.height;

        RDE_CORE_INFO("Creating window {0} ({1}, {2})", window_config.title, window_config.width, window_config.height);

        if (!s_glfw_initialized) {
            int success = glfwInit();
            RDE_CORE_ASSERT(success, "Could not initialize GLFW!");

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

        glfwSetDropCallback(m_window, [](GLFWwindow *window, int count, const char **filepaths) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            std::vector<std::string> files;
            files.reserve(count);
            for (int i = 0; i < count; ++i) {
                files.emplace_back(filepaths[i]);
            }
            WindowFileDropEvent event(files);
            data.event_callback(event);
        });
    }

    void GlfwOpenGLWindow::shutdown() {
        glfwDestroyWindow(m_window);
    }

    void GlfwOpenGLWindow::on_update() {
        glfwPollEvents();
        glfwSwapBuffers(m_window);
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
}
