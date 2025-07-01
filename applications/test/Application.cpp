#include "Application.h"
#include "events/ApplicationEvent.h"
#include "events/MouseEvent.h"
#include "events/KeyEvent.h"
#include "Log.h"
#include "GetAssetPath.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace RDE {
    Application::Application() {
        Log::Initialize();
        m_app_context = std::make_shared<ApplicationContext>();
        m_app_context->m_registry = std::make_shared<entt::registry>();
        m_app_context->m_dispatcher = std::make_shared<entt::dispatcher>();
    }

    Application::~Application() {
        // Cleanup if necessary
        m_app_context.reset();
    }

    static bool s_glfw_initialized = false;

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    bool Application::init(int width, int height, const char *title) {
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

        m_app_context->m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!m_app_context->m_window) {
            throw std::runtime_error("Failed to create GLFW window");
        }
        glfwMakeContextCurrent(m_app_context->m_window);
        // Initialize other components and start the main loop

        m_app_context->m_width = width;
        m_app_context->m_height = height;
        m_app_context->m_title = title;
        m_app_context->m_is_running = true;
        m_app_context->m_is_minimized = false;
        m_app_context->m_primary_camera_entity = m_app_context->m_registry->create();
        m_app_context->m_last_selected_entity = entt::null; // No entity selected initially
        m_app_context->m_selected_entities.clear();
        m_app_context->m_event_callback = [this](Event &e) {
            this->on_event(e);
        };
        // Main loop
        glfwSetWindowUserPointer(m_app_context->m_window, this); // Set the user pointer to this Application instance

        // Set up event callbacks
        glfwSetWindowSizeCallback(m_app_context->m_window, [](GLFWwindow *window, int width, int height) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();
            app_context.m_width = width;
            app_context.m_height = height;
            app_context.m_is_minimized = (width == 0 || height == 0);

            WindowResizeEvent event(width, height);
            if (app_context.m_event_callback) {
                app_context.m_event_callback(event);
            }
        });

        glfwSetWindowCloseCallback(m_app_context->m_window, [](GLFWwindow *window) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();
            app_context.m_is_running = false; // Stop the main loop

            WindowCloseEvent event;
            if (app_context.m_event_callback) {
                app_context.m_event_callback(event);
            }
        });

        glfwSetKeyCallback(m_app_context->m_window,
                           [](GLFWwindow *window, int key, int scancode, int action, int mods) {
                               auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
                               auto &app_context = app->get_app_context();

                               // Handle key events
                               if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                                   app_context.m_is_running = false; // Stop the main loop on Escape key press
                               }

                               switch (action) {
                                   case GLFW_PRESS: {
                                       app_context.m_keyboard_state.keys_pressed_current_frame[key] = true;
                                       app_context.m_keyboard_state.keys_held_this_frame.insert(key);
                                       KeyPressedEvent event(key, 0);
                                       if (app_context.m_event_callback) {
                                           app_context.m_event_callback(event);
                                       }

                                       // Check if the key is bound to an action
                                       if (app_context.m_key_press_bindings.find(key) !=
                                           app_context.m_key_press_bindings.end()) {
                                           app_context.m_key_press_bindings[key]();
                                       }
                                       break;
                                   }
                                   case GLFW_RELEASE: {
                                       app_context.m_keyboard_state.keys_pressed_current_frame[key] = false;
                                       app_context.m_keyboard_state.keys_held_this_frame.erase(key);
                                       KeyReleasedEvent event(key);
                                       if (app_context.m_event_callback) {
                                           app_context.m_event_callback(event);
                                       }

                                       // Check if the key is bound to an action
                                       if (app_context.m_key_release_bindings.find(key) !=
                                           app_context.m_key_release_bindings.end()) {
                                           app_context.m_key_release_bindings[key]();
                                       }
                                       break;
                                   }
                                   case GLFW_REPEAT: {
                                       app_context.m_keyboard_state.keys_repeated[key] = true;
                                       app_context.m_keyboard_state.keys_held_this_frame.insert(key);

                                       KeyPressedEvent event(key, 1);
                                       if (app_context.m_event_callback) {
                                           app_context.m_event_callback(event);
                                       }

                                       // Check if the key is bound to an action
                                       if (app_context.m_key_repeat_bindings.find(key) !=
                                           app_context.m_key_repeat_bindings.end()) {
                                           app_context.m_key_repeat_bindings[key]();
                                       }
                                       break;
                                   }
                               }
                           });

        glfwSetMouseButtonCallback(m_app_context->m_window, [](GLFWwindow *window, int button, int action, int mods) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();

            switch (action) {
                case GLFW_PRESS: {
                    app_context.m_mouse_state.buttons_current_frame[button].button = button;
                    app_context.m_mouse_state.buttons_current_frame[button].is_pressed = true;
                    app_context.m_mouse_state.buttons_current_frame[button].is_released = false;
                    app_context.m_mouse_state.buttons_current_frame[button].press_position = app_context.m_mouse_state.cursor_position;

                    MouseButtonPressedEvent event(button);
                    if (app_context.m_event_callback) {
                        app_context.m_event_callback(event);
                    }
                    break;
                }
                case GLFW_RELEASE: {
                    app_context.m_mouse_state.buttons_current_frame[button].button = button;
                    app_context.m_mouse_state.buttons_current_frame[button].is_pressed = false;
                    app_context.m_mouse_state.buttons_current_frame[button].is_released = true;
                    app_context.m_mouse_state.buttons_current_frame[button].release_position = app_context.m_mouse_state.cursor_position;

                    MouseButtonReleasedEvent event(button);
                    if (app_context.m_event_callback) {
                        app_context.m_event_callback(event);
                    }
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_app_context->m_window, [](GLFWwindow *window, double x_offset, double y_offset) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();

            // Update mouse scroll state
            app_context.m_mouse_state.scroll_offset.x += (float) x_offset;
            app_context.m_mouse_state.scroll_offset.y += (float) y_offset;
            app_context.m_mouse_state.scroll_delta.x = (float) x_offset;
            app_context.m_mouse_state.scroll_delta.y = (float) y_offset;

            MouseScrolledEvent event((float) x_offset, (float) y_offset);
            if (app_context.m_event_callback) {
                app_context.m_event_callback(event);
            }
        });

        glfwSetCursorPosCallback(m_app_context->m_window, [](GLFWwindow *window, double x_pos, double y_pos) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();

            glm::vec2 last_position = app_context.m_mouse_state.cursor_position;
            app_context.m_mouse_state.cursor_position = glm::vec2(x_pos, y_pos);
            app_context.m_mouse_state.delta_position = app_context.m_mouse_state.cursor_position - last_position;
            app_context.m_mouse_state.is_moving = glm::length(app_context.m_mouse_state.delta_position) > 0.0f;
            app_context.m_mouse_state.is_dragging = app_context.m_mouse_state.any_pressed();

            MouseMovedEvent event((float) x_pos, (float) y_pos);
            if (app_context.m_event_callback) {
                app_context.m_event_callback(event);
            }
        });

        glfwSetDropCallback(m_app_context->m_window, [](GLFWwindow *window, int count, const char **filepaths) {
            auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
            auto &app_context = app->get_app_context();

            std::vector<std::string> files;
            files.reserve(count);
            for (int i = 0; i < count; ++i) {
                files.emplace_back(filepaths[i]);
            }
            WindowFileDropEvent event(files);
            if (app_context.m_event_callback) {
                app_context.m_event_callback(event);
            }
        });

        {
            // Initialize OpenGL context
            int version = gladLoadGL(glfwGetProcAddress);
            // Initialize GLAD (or your OpenGL function loader)
            if (version == 0) {
                throw std::runtime_error("Failed to initialize GLAD");
            }

            RDE_CORE_INFO("OpenGL Info: Vendor: {}, Renderer: {}, Version: {}",
                          (const char *) glGetString(GL_VENDOR),
                          (const char *) glGetString(GL_RENDERER),
                          (const char *) glGetString(GL_VERSION));


            // Set viewport and clear color
            glViewport(0, 0, width, height);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

            // Enable depth testing
            glEnable(GL_DEPTH_TEST);
        }

        {
            // Initialize ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable multi-viewport support

            ImGui::StyleColorsDark(); // Set dark theme

            ImGuiStyle &style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            ImGui_ImplGlfw_InitForOpenGL(m_app_context->m_window, true);
            ImGui_ImplOpenGL3_Init("#version 410");
        }

        {
            m_app_context->m_asset_database = std::make_shared<AssetDatabase>();
            m_app_context->m_asset_manager = std::make_unique<AssetManager>(*m_app_context->m_asset_database);
            m_app_context->m_file_watcher = std::make_unique<FileWatcher>();
            m_app_context->m_file_watcher_event_queue = std::make_unique<ThreadSafeQueue<std::string>>();
            auto path =  get_asset_path();

            m_app_context->m_file_watcher->start(path->string(), m_app_context->m_file_watcher_event_queue.get());
        }
        return true;
    }

    void Application::shutdown() {
        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_detach(*m_app_context);
        }
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        if (m_app_context->m_window) {
            glfwDestroyWindow(m_app_context->m_window);
            m_app_context->m_window = nullptr;
        }
        glfwTerminate();
    }

    void Application::run(int width, int height, const char *title) {
        if(!init(width, height, title)) {
            throw std::runtime_error("Failed to initialize the application");
        }

        while (!glfwWindowShouldClose(m_app_context->m_window) && m_app_context->m_is_running) {
            // Poll events
            glfwPollEvents();

            // Check if the window is minimized
            int width, height;
            glfwGetFramebufferSize(m_app_context->m_window, &width, &height);
            if (width == 0 || height == 0) {
                m_app_context->m_is_minimized = true;
                continue; // Skip rendering if minimized
            } else {
                m_app_context->m_is_minimized = false;
            }

            // Render and update logic here
            on_update();

            on_render();

            on_render_gui();

            // Swap buffers
            glfwSwapBuffers(m_app_context->m_window);
        }

        shutdown();
    }

    void Application::on_update() {
        for (const auto key: m_app_context->m_keyboard_state.keys_held_this_frame) {
            if (m_app_context->m_key_update_bindings.find(key) != m_app_context->m_key_update_bindings.end()) {
                m_app_context->m_key_update_bindings[key]();
            }
        }
        while(!m_app_context->m_file_watcher_event_queue->empty()) {
            auto file_path_opt = m_app_context->m_file_watcher_event_queue->try_pop();
            if (file_path_opt) {
                const std::string &file_path = *file_path_opt;
                // Handle file change event, e.g., reload assets
                m_app_context->m_asset_manager->force_load_from(file_path);
                RDE_CORE_INFO("File reloaded: {}", file_path);
            }
        }

        // Update layers
        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_update(*m_app_context);
        }

        // After all updates, reset the current frame states
        m_app_context->m_keyboard_state.keys_pressed_last_frame = m_app_context->m_keyboard_state.keys_pressed_current_frame;
        m_app_context->m_keyboard_state.keys_repeated = std::vector<bool>(
                m_app_context->m_keyboard_state.keys_pressed_current_frame.size(), false);
        m_app_context->m_mouse_state.scroll_delta = {0.0f, 0.0f};
        m_app_context->m_mouse_state.buttons_last_frame = m_app_context->m_mouse_state.buttons_current_frame;
        m_app_context->m_mouse_state.buttons_current_frame = std::vector<Mouse::Button>(
                m_app_context->m_mouse_state.buttons_current_frame.size(), Mouse::Button{});
    }

    void Application::on_render() {
        // Render logic here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Additional rendering code...

        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_render(*m_app_context);
        }
    }

    void Application::on_render_gui() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginMainMenuBar();

        // Render ImGui UI here
        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_render_gui(*m_app_context);
        }

        ImGui::EndMainMenuBar();
        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) m_app_context->m_width, (float) m_app_context->m_height);

        ImGui::Render();
        //this is now handled in the imgui pass

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void Application::on_event(Event &e) {
        // Handle events here

        ImGuiIO &io = ImGui::GetIO();
        e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

        if (!e.handled) {
            for (auto it = m_app_context->m_layer_stack.rbegin(); it != m_app_context->m_layer_stack.rend(); ++it) {
                if (e.handled) {
                    break;
                }
                (*it)->on_event(e, *m_app_context);
            }
        }
    }
}

int main() {
    RDE::Application app;
    try {
        app.run(1280, 720, "RDE Application");
    } catch (const std::exception& e) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}