#include "SandboxApp.h"
#include "core/EntryPoint.h"
#include "core/events/ApplicationEvent.h"
#include "core/events/MouseEvent.h"
#include "core/events/KeyEvent.h"
#include "core/Log.h"
#include "core/Paths.h"

#include "core/Ticker.h"
#include "systems/TransformSystem.h"
#include "systems/CameraSystem.h"
#include "systems/HierarchySystem.h"
#include "systems/BoundingVolumeSystem.h"

#include "src/GlfwVulkanWindow.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace RDE {
    SandboxApp::SandboxApp(std::unique_ptr<IWindow> window) : m_window(std::move(window)) {
        Log::Initialize();
        m_window->set_event_callback(RDE_BIND_EVENT_FN(SandboxApp::on_event));
        m_app_context = std::make_shared<ApplicationContext>();
        m_app_context->m_window = m_window.get();
        m_app_context->m_native_window = static_cast<GLFWwindow *>(m_window->get_native_handle());

        m_app_context->m_registry = std::make_shared<entt::registry>();
        m_app_context->m_dispatcher = std::make_shared<entt::dispatcher>();
        m_app_context->m_system_scheduler = std::make_unique<SystemScheduler>(*m_app_context->m_registry);
    }

    SandboxApp::~SandboxApp() {
        // Cleanup if necessary
        m_app_context.reset();
    }

    static bool s_glfw_initialized = false;

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    bool SandboxApp::init(int width, int height, const char *title) {
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
        glfwMakeContextCurrent(m_app_context->m_native_window);
        // Initialize other components and start the main loop


        m_app_context->m_is_running = true;
        m_app_context->m_is_minimized = false;
        m_app_context->m_primary_camera_entity = m_app_context->m_registry->create();
        m_app_context->m_last_selected_entity = entt::null; // No entity selected initially
        m_app_context->m_selected_entities.clear();
        m_app_context->m_mouse_state.buttons_current_frame.resize(3);
        m_app_context->m_mouse_state.buttons_last_frame.resize(3);
        m_app_context->m_keyboard_state.keys_pressed_current_frame.resize(GLFW_KEY_LAST + 1, false);
        m_app_context->m_keyboard_state.keys_pressed_last_frame.resize(GLFW_KEY_LAST + 1, false);
        m_app_context->m_keyboard_state.keys_repeated.resize(GLFW_KEY_LAST + 1, false);

        glfwSetWindowUserPointer(m_app_context->m_native_window, this);
        // Set the user pointer to this SandboxApp instance

        // Set up event callbacks

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
        } {
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

            ImGui_ImplGlfw_InitForOpenGL(m_app_context->m_native_window, true);
            ImGui_ImplOpenGL3_Init("#version 410");
        } {
            m_app_context->m_asset_database = std::make_shared<AssetDatabase>();
            m_app_context->m_asset_manager = std::make_unique<AssetManager>(*m_app_context->m_asset_database);
            m_app_context->m_file_watcher = std::make_unique<FileWatcher>();
            m_app_context->m_file_watcher_event_queue = std::make_unique<ThreadSafeQueue<std::string> >();
            auto path = get_asset_path();

            m_app_context->m_file_watcher->start(path->string(), m_app_context->m_file_watcher_event_queue.get());
        } {
            auto &registry = *m_app_context->m_registry;
            m_app_context->m_system_scheduler->register_system<HierarchySystem>(registry);
            m_app_context->m_system_scheduler->register_system<TransformSystem>(registry);
            m_app_context->m_system_scheduler->register_system<BoundingVolumeSystem>(registry);
            m_app_context->m_system_scheduler->register_system<CameraSystem>(registry);
            RDE_CORE_INFO("Registered systems: HierarchySystem, TransformSystem, BoundingVolumeSystem, CameraSystem");
            m_app_context->m_system_scheduler->bake();
            RDE_CORE_INFO("SystemScheduler baked successfully");
        }
        return true;
    }

    void SandboxApp::shutdown() {
        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_detach();
        } {
            m_app_context->m_file_watcher->stop();
            m_app_context->m_file_watcher.reset();
            m_app_context->m_file_watcher_event_queue.reset();
            m_app_context->m_asset_manager.reset();
            m_app_context->m_asset_database.reset();
        } {
            m_app_context->m_system_scheduler->shutdown();
            m_app_context->m_system_scheduler.reset();
        } {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        if (m_app_context->m_window) {
            glfwDestroyWindow(m_app_context->m_native_window);
            m_app_context->m_window = nullptr;
        }
        glfwTerminate();
    }

    void SandboxApp::run(int width, int height, const char *title) {
        if (!init(width, height, title)) {
            throw std::runtime_error("Failed to initialize the application");
        }

        Ticker timer;
        while (!glfwWindowShouldClose(m_app_context->m_native_window) && m_app_context->m_is_running) {
            // Poll events
            glfwPollEvents();

            // Check if the window is minimized
            int width, height;
            glfwGetFramebufferSize(m_app_context->m_native_window, &width, &height);
            if (width == 0 || height == 0) {
                m_app_context->m_is_minimized = true;
                continue; // Skip rendering if minimized
            } else {
                m_app_context->m_is_minimized = false;
            }

            float delta_time = timer.tick();
            // Render and update logic here
            on_update(delta_time);

            on_render();

            on_render_gui();

            // Swap buffers
            glfwSwapBuffers(m_app_context->m_native_window);
        }

        shutdown();
    }

    void SandboxApp::on_update(float delta_time) {
        for (const auto key: m_app_context->m_keyboard_state.keys_held_this_frame) {
            if (m_app_context->m_key_update_bindings.find(key) != m_app_context->m_key_update_bindings.end()) {
                m_app_context->m_key_update_bindings[key]();
            }
        }
        while (!m_app_context->m_file_watcher_event_queue->empty()) {
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
            layer->on_update(delta_time);
        }

        m_app_context->m_system_scheduler->execute(delta_time);

        // After all updates, reset the current frame states
        m_app_context->m_keyboard_state.keys_pressed_last_frame = m_app_context->m_keyboard_state.
                keys_pressed_current_frame;
        m_app_context->m_keyboard_state.keys_repeated = std::vector<bool>(
            m_app_context->m_keyboard_state.keys_pressed_current_frame.size(), false);
        m_app_context->m_mouse_state.scroll_delta = {0.0f, 0.0f};
        m_app_context->m_mouse_state.buttons_last_frame = m_app_context->m_mouse_state.buttons_current_frame;
        m_app_context->m_mouse_state.buttons_current_frame = std::vector<Mouse::Button>(
            m_app_context->m_mouse_state.buttons_current_frame.size(), Mouse::Button{});
    }

    void SandboxApp::on_render() {
        // Render logic here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Additional rendering code...

        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_render();
        }
    }

    void SandboxApp::on_render_gui() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginMainMenuBar();

        // Render ImGui UI here
        for (auto &layer: m_app_context->m_layer_stack) {
            layer->on_render_gui();
        }

        ImGui::EndMainMenuBar();
        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) m_app_context->m_window->get_width(),
                                (float) m_app_context->m_window->get_height());

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

    void SandboxApp::on_event(Event &e) {
        // Handle events here

        ImGuiIO &io = ImGui::GetIO();
        e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

        if (!e.handled) {
            EventDispatcher dispatcher(e);
            dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &) {
                m_app_context->m_is_running = false;
                return true;
            });
            dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) {
                // Don't process if minimized
                if (e.get_width() == 0 || e.get_height() == 0) {
                    m_app_context->m_is_minimized = true;
                    return false;
                }
                m_app_context->m_is_minimized = false;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent &e) {
                auto key = e.get_key_code();
                if (e.is_repeat()) {
                    m_app_context->m_keyboard_state.keys_repeated[key] = true;
                    m_app_context->m_keyboard_state.keys_held_this_frame.insert(key);

                    // Check if the key is bound to an repeat action
                    if (m_app_context->m_key_repeat_bindings.find(key) != m_app_context->m_key_repeat_bindings.end()) {
                        m_app_context->m_key_repeat_bindings[key]();
                    }
                } else {
                    m_app_context->m_keyboard_state.keys_pressed_current_frame[key] = true;
                    m_app_context->m_keyboard_state.keys_held_this_frame.insert(key);

                    // Check if the key is bound to a single press action
                    if (m_app_context->m_key_press_bindings.find(key) != m_app_context->m_key_press_bindings.end()) {
                        m_app_context->m_key_press_bindings[key]();
                    }
                }
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<KeyReleasedEvent>([this](KeyReleasedEvent &e) {
                auto key = e.get_key_code();
                m_app_context->m_keyboard_state.keys_pressed_current_frame[key] = false;
                m_app_context->m_keyboard_state.keys_held_this_frame.erase(key);

                // Check if the key is bound to a release action
                if (m_app_context->m_key_release_bindings.find(key) != m_app_context->m_key_release_bindings.end()) {
                    m_app_context->m_key_release_bindings[key]();
                }
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e) {
                auto button = e.get_mouse_button();
                m_app_context->m_mouse_state.buttons_current_frame[button].is_pressed = true;
                m_app_context->m_mouse_state.buttons_current_frame[button].is_released = false;
                m_app_context->m_mouse_state.buttons_current_frame[button].button = button;
                m_app_context->m_mouse_state.buttons_current_frame[button].press_position =
                        m_app_context->m_mouse_state.cursor_position;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent &e) {
                auto button = e.get_mouse_button();
                m_app_context->m_mouse_state.buttons_current_frame[button].is_pressed = false;
                m_app_context->m_mouse_state.buttons_current_frame[button].is_released = true;
                m_app_context->m_mouse_state.buttons_current_frame[button].button = button;
                m_app_context->m_mouse_state.buttons_current_frame[button].release_position =
                        m_app_context->m_mouse_state.cursor_position;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &e) {
                glm::vec2 last_position = m_app_context->m_mouse_state.cursor_position;
                m_app_context->m_mouse_state.cursor_position = glm::vec2(e.get_x(), e.get_y());
                m_app_context->m_mouse_state.delta_position =
                        m_app_context->m_mouse_state.cursor_position - last_position;
                m_app_context->m_mouse_state.is_moving =
                        glm::length(m_app_context->m_mouse_state.delta_position) > 0.0f;
                m_app_context->m_mouse_state.is_dragging = m_app_context->m_mouse_state.any_pressed();
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &e) {
                m_app_context->m_mouse_state.scroll_offset.x += e.get_x_offset();
                m_app_context->m_mouse_state.scroll_offset.y += e.get_y_offset();
                m_app_context->m_mouse_state.scroll_delta.x = e.get_x_offset();
                m_app_context->m_mouse_state.scroll_delta.y = e.get_y_offset();
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<WindowFileDropEvent>([this](WindowFileDropEvent &e) {
                // Handle file drop event
                for (const auto &file_path: e.get_files()) {
                    m_app_context->m_asset_manager->force_load_from(file_path);
                    RDE_CORE_INFO("File dropped: {}", file_path);
                }
                return false; // Allow layers to handle the event
            });

            for (auto it = m_app_context->m_layer_stack.rbegin(); it != m_app_context->m_layer_stack.rend(); ++it) {
                if (e.handled) {
                    break;
                }
                (*it)->on_event(e);
            }
        }
    }

    Application *CreateApplication() {
        //Here provide all dependencies to inject into the application
        auto window = GlfwVulkanWindow::Create();
        return new SandboxApp(std::move(window));
    }
}


int main() {
    RDE::Application *app = RDE::CreateApplication();
    try {
        app->run(1280, 720, "RDE SandboxApp");
    } catch (const std::exception &e) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
