#include "GlfwVulkanWindow.h"
#include "core/Log.h"
#include "core/KeyCodes.h"
#include "core/events/ApplicationEvent.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

#include <GLFW/glfw3.h>

#include "core/Mouse.h"


namespace RDE::Detail {
    // Use a detail namespace to hide helpers

    // The lookup table. It's large but that's okay, it's static and initialized once.
    // GLFW_KEY_LAST is ~348, so this array is small.
    static std::array<KeyCode, GLFW_KEY_LAST + 1> s_glfw_to_rde_key_map;

    // A helper function to build the map once on startup.
    static void initialize_key_map() {
        // First, initialize all keys to Unknown
        s_glfw_to_rde_key_map.fill(KeyCode::KEY_UNKNOWN);

        // Map all the special keys explicitly
        s_glfw_to_rde_key_map[GLFW_KEY_SPACE] = KeyCode::KEY_SPACE;
        s_glfw_to_rde_key_map[GLFW_KEY_APOSTROPHE] = KeyCode::KEY_APOSTROPHE;
        s_glfw_to_rde_key_map[GLFW_KEY_COMMA] = KeyCode::KEY_COMMA;
        s_glfw_to_rde_key_map[GLFW_KEY_MINUS] = KeyCode::KEY_MINUS;
        s_glfw_to_rde_key_map[GLFW_KEY_PERIOD] = KeyCode::KEY_PERIOD;
        s_glfw_to_rde_key_map[GLFW_KEY_SLASH] = KeyCode::KEY_SLASH;
        // ... other punctuation ...

        // Use loops for character and number ranges to save typing
        for (int i = 0; i < 26; ++i) {
            s_glfw_to_rde_key_map[GLFW_KEY_A + i] = static_cast<KeyCode>(static_cast<int>(RDE::KeyCode::KEY_A) + i);
        }
        for (int i = 0; i < 10; ++i) {
            s_glfw_to_rde_key_map[GLFW_KEY_0 + i] = static_cast<KeyCode>(static_cast<int>(RDE::KeyCode::KEY_0) + i);
        }

        // Map function keys
        s_glfw_to_rde_key_map[GLFW_KEY_ESCAPE] = KeyCode::KEY_ESCAPE;
        s_glfw_to_rde_key_map[GLFW_KEY_ENTER] = KeyCode::KEY_ENTER;
        s_glfw_to_rde_key_map[GLFW_KEY_TAB] = KeyCode::KEY_TAB;
        s_glfw_to_rde_key_map[GLFW_KEY_BACKSPACE] = KeyCode::KEY_BACKSPACE;
        s_glfw_to_rde_key_map[GLFW_KEY_LEFT_SHIFT] = KeyCode::KEY_LEFT_SHIFT;
        s_glfw_to_rde_key_map[GLFW_KEY_LEFT_CONTROL] = KeyCode::KEY_LEFT_CONTROL;
        s_glfw_to_rde_key_map[GLFW_KEY_LEFT_ALT] = KeyCode::KEY_LEFT_ALT;
        s_glfw_to_rde_key_map[GLFW_KEY_LEFT_SUPER] = KeyCode::KEY_LEFT_SUPER;
        s_glfw_to_rde_key_map[GLFW_KEY_RIGHT_SHIFT] = KeyCode::KEY_RIGHT_SHIFT;
        s_glfw_to_rde_key_map[GLFW_KEY_RIGHT_CONTROL] = KeyCode::KEY_RIGHT_CONTROL;
        s_glfw_to_rde_key_map[GLFW_KEY_RIGHT_ALT] = KeyCode::KEY_RIGHT_ALT;
        s_glfw_to_rde_key_map[GLFW_KEY_RIGHT_SUPER] = KeyCode::KEY_RIGHT_SUPER;
        s_glfw_to_rde_key_map[GLFW_KEY_MENU] = KeyCode::KEY_MENU;

        // ... map ALL other keys you care about (arrows, F-keys, etc.) ...
        for (int i = 0; i <= GLFW_KEY_F25 - GLFW_KEY_F1; ++i) {
            s_glfw_to_rde_key_map[i] = static_cast<KeyCode>(static_cast<int>(KeyCode::KEY_F1) + i);
        }

        // Keypad keys
        s_glfw_to_rde_key_map[GLFW_KEY_KP_0] = KeyCode::KEY_KP_0;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_1] = KeyCode::KEY_KP_1;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_2] = KeyCode::KEY_KP_2;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_3] = KeyCode::KEY_KP_3;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_4] = KeyCode::KEY_KP_4;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_5] = KeyCode::KEY_KP_5;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_6] = KeyCode::KEY_KP_6;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_7] = KeyCode::KEY_KP_7;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_8] = KeyCode::KEY_KP_8;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_9] = KeyCode::KEY_KP_9;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_DECIMAL] = KeyCode::KEY_KP_DECIMAL;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_DIVIDE] = KeyCode::KEY_KP_DIVIDE;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_MULTIPLY] = KeyCode::KEY_KP_MULTIPLY;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_SUBTRACT] = KeyCode::KEY_KP_SUBTRACT;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_ADD] = KeyCode::KEY_KP_ADD;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_ENTER] = KeyCode::KEY_KP_ENTER;
        s_glfw_to_rde_key_map[GLFW_KEY_KP_EQUAL] = KeyCode::KEY_KP_EQUAL;
        // ... and so on for all other keys you care about ...
    }

    // The public-facing (within this file) mapping function
    inline KeyCode to_rde_key_code(int glfw_key) {
        if (glfw_key >= 0 && glfw_key <= GLFW_KEY_LAST) {
            return s_glfw_to_rde_key_map[glfw_key];
        }
        return RDE::KeyCode::KEY_UNKNOWN;
    }

    static std::array<MouseButton, GLFW_MOUSE_BUTTON_8 + 1> s_glfw_to_rde_button_map;

    static void initialize_button_map() {
        s_glfw_to_rde_button_map[GLFW_MOUSE_BUTTON_LEFT] = MouseButton::BUTTON_LEFT;
        s_glfw_to_rde_button_map[GLFW_MOUSE_BUTTON_RIGHT] = MouseButton::BUTTON_RIGHT;
        s_glfw_to_rde_button_map[GLFW_MOUSE_BUTTON_MIDDLE] = MouseButton::BUTTON_MIDDLE;
        // Map additional mouse buttons if needed
    }

    inline MouseButton to_rde_button_code(int glfw_button) {
        if (glfw_button >= 0 && glfw_button <= GLFW_MOUSE_BUTTON_8 + 1) {
            return s_glfw_to_rde_button_map[glfw_button];
        }
        return MouseButton::BUTTON_UNKNOWN;
    }
} // namespace RDE::Detail```
namespace RDE {
    // Some static helper for initializing GLFW once
    static void InitializeGLFW() {
        static bool is_initialized = false;
        if (!is_initialized) {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW!");
            }
            // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            is_initialized = true;
            Detail::initialize_key_map();
            Detail::initialize_button_map();
        }
    }

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    std::unique_ptr<IWindow> IWindow::Create(const WindowConfig &config) {
        return std::make_unique<GlfwVulkanWindow>(config);
    }

    GlfwVulkanWindow::GlfwVulkanWindow(const WindowConfig &window_config) {
        m_data.title = window_config.title;
        m_data.width = window_config.width;
        m_data.height = window_config.height;

        InitializeGLFW();
        glfwSetErrorCallback(GlfwErrorCallback);

        m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window!");
        }

        glfwSetWindowUserPointer(m_window, &m_data); // Set the user pointer to this Application instance

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

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action,
                                        [[maybe_unused]] int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(Detail::to_rde_key_code(key), 0);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(Detail::to_rde_key_code(key));
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(Detail::to_rde_key_code(key), 1);
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, [[maybe_unused]] int mods) {
            WindowData &data = *(WindowData *) glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(Detail::to_rde_button_code(button));
                    if (data.event_callback) {
                        data.event_callback(event);
                    }
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(Detail::to_rde_button_code(button));
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
    }

    GlfwVulkanWindow::~GlfwVulkanWindow() {
        shutdown();
    }

    void GlfwVulkanWindow::poll_events() {
        glfwPollEvents();
    }

    void GlfwVulkanWindow::set_vsync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        m_data.vsync = enabled;
    }

    void GlfwVulkanWindow::get_framebuffer_size(int &width, int &height) const {
        glfwGetFramebufferSize(m_window, &width, &height);
    }

    void GlfwVulkanWindow::get_window_content_scale(float &xscale, float &yscale) const {
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        if (xscale <= 0.f) xscale = 1.f; // Prevent division by zero
        if (yscale <= 0.f) yscale = 1.f; // Prevent division by zero
    }

    bool GlfwVulkanWindow::should_close() {
        return glfwWindowShouldClose(m_window);
    }

    void GlfwVulkanWindow::shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void GlfwVulkanWindow::terminate() {
        shutdown();
        glfwTerminate();
    }

    void GlfwVulkanWindow::swap_buffers() {
        glfwSwapBuffers(m_window);
    }
}
