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

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include "VulkanHelper.h"

#include <GLFW/glfw3.h>

#include "ImGuiLayer.h"

#include <imgui.h>

namespace RDE {
    SandboxApp::SandboxApp(std::unique_ptr<IWindow> window) : m_window(
            std::move(window)) {
        Log::Initialize();
        m_window->set_event_callback(RDE_BIND_EVENT_FN(SandboxApp::on_event));
        m_renderer = std::make_unique<Renderer>(m_window.get());


        m_registry = std::make_shared<entt::registry>();
        m_dispatcher = std::make_shared<entt::dispatcher>();
        m_system_scheduler = std::make_unique<SystemScheduler>(*m_registry);
    }

    SandboxApp::~SandboxApp() {
        // Cleanup if necessary

    }

    static void GlfwErrorCallback(int error, const char *description) {
        RDE_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

    bool SandboxApp::init(int width, int height, const char *title) {
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        m_is_running = true;
        m_is_minimized = false;

        m_primary_camera_entity = m_registry->create();
        m_last_selected_entity = entt::null; // No entity selected initially
        m_selected_entities.clear();

        m_mouse_state.buttons_current_frame.resize(3);
        m_mouse_state.buttons_last_frame.resize(3);

        m_keyboard_state.keys_pressed_current_frame.resize(GLFW_KEY_LAST + 1, false);
        m_keyboard_state.keys_pressed_last_frame.resize(GLFW_KEY_LAST + 1, false);
        m_keyboard_state.keys_repeated.resize(GLFW_KEY_LAST + 1, false);

        {
            m_asset_database = std::make_shared<AssetDatabase>();
            m_asset_manager = std::make_unique<AssetManager>(*m_asset_database);
            m_file_watcher = std::make_unique<FileWatcher>();
            m_file_watcher_event_queue = std::make_unique<ThreadSafeQueue<std::string> >();
            auto path = get_asset_path();

            m_file_watcher->start(path->string(), m_file_watcher_event_queue.get());
        }
        {
            m_system_scheduler->register_system<HierarchySystem>(*m_registry);
            m_system_scheduler->register_system<TransformSystem>(*m_registry);
            m_system_scheduler->register_system<BoundingVolumeSystem>(*m_registry);
            m_system_scheduler->register_system<CameraSystem>(*m_registry);
            RDE_CORE_INFO("Registered systems: HierarchySystem, TransformSystem, BoundingVolumeSystem, CameraSystem");
            m_system_scheduler->bake();
            RDE_CORE_INFO("SystemScheduler baked successfully");
        }

        m_renderer->init();

        auto imgui_layer = std::make_shared<ImGuiLayer>(m_window.get(),
                                                    m_renderer->get_device());
        m_imgui_layer = imgui_layer.get();
        m_layer_stack.push_overlay(imgui_layer); // Assuming you have a push_layer method
        return true;
    }

    void SandboxApp::shutdown() {
        for (auto &layer: m_layer_stack) {
            layer->on_detach();
        }
        {
            m_file_watcher->stop();
            m_file_watcher.reset();
            m_file_watcher_event_queue.reset();
            m_asset_manager.reset();
            m_asset_database.reset();
        }
        {
            m_system_scheduler->shutdown();
            m_system_scheduler.reset();
        }
        if (m_window) {
            m_window.reset();
        }
        glfwTerminate();
    }

    void SandboxApp::run(int width, int height, const char *title) {
        if (!init(width, height, title)) {
            throw std::runtime_error("Failed to initialize the application");
        }

        Ticker timer;
        auto *glfw_window = static_cast<GLFWwindow *>(m_window->get_native_handle());
        while (!m_window->should_close() && m_is_running) {
            // Poll events
            m_window->poll_events();

            // Check if the window is minimized
            int width, height;
            glfwGetFramebufferSize(glfw_window, &width, &height);
            if (width == 0 || height == 0) {
                m_is_minimized = true;
                continue; // Skip rendering if minimized
            } else {
                m_is_minimized = false;
            }

            float delta_time = timer.tick();
            // Render and update logic here
            on_update(delta_time);

            on_render();
        }

        shutdown();
    }

    void SandboxApp::on_update(float delta_time) {
        for (const auto key: m_keyboard_state.keys_held_this_frame) {
            if (m_key_update_bindings.find(key) != m_key_update_bindings.end()) {
                m_key_update_bindings[key]();
            }
        }
        while (!m_file_watcher_event_queue->empty()) {
            auto file_path_opt = m_file_watcher_event_queue->try_pop();
            if (file_path_opt) {
                const std::string &file_path = *file_path_opt;
                // Handle file change event, e.g., reload assets
                m_asset_manager->force_load_from(file_path);
                RDE_CORE_INFO("File reloaded: {}", file_path);
            }
        }

        // Update layers
        for (auto &layer: m_layer_stack) {
            layer->on_update(delta_time);
        }

        m_system_scheduler->execute(delta_time);

        // After all updates, reset the current frame states
        m_keyboard_state.keys_pressed_last_frame = m_keyboard_state.
                keys_pressed_current_frame;
        m_keyboard_state.keys_repeated = std::vector<bool>(
                m_keyboard_state.keys_pressed_current_frame.size(), false);
        m_mouse_state.scroll_delta = {0.0f, 0.0f};
        m_mouse_state.buttons_last_frame = m_mouse_state.buttons_current_frame;
        m_mouse_state.buttons_current_frame = std::vector<Mouse::Button>(
                m_mouse_state.buttons_current_frame.size(), Mouse::Button{});
    }

    void SandboxApp::on_render() {
        // Render logic here
/*        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
        // Additional rendering code...

        auto device = m_renderer->get_device();
        if (RAL::CommandBuffer *cmd = device->begin_frame()) {

            // --- 1. Main Scene Render Pass ---
            // For now, we are just clearing the screen. In the future, you would
            // render your 3D scene here inside a render pass.
            RAL::RenderPassDescription passDesc{};
            passDesc.colorAttachments.resize(1);
            passDesc.colorAttachments[0].texture = RAL::TextureHandle::INVALID(); // INVALID handle means use the swapchain
            passDesc.colorAttachments[0].loadOp = RAL::LoadOp::Clear;
            passDesc.colorAttachments[0].storeOp = RAL::StoreOp::Store;
            passDesc.colorAttachments[0].clearColor[0] = 0.1f;
            passDesc.colorAttachments[0].clearColor[1] = 0.1f;
            passDesc.colorAttachments[0].clearColor[2] = 0.15f;
            passDesc.colorAttachments[0].clearColor[3] = 1.0f;

            cmd->begin_render_pass(passDesc);

            // 2. Let your scene layers render themselves (e.g., the 3D world)
            for (auto &layer: m_layer_stack) {
                if (layer.get() != m_imgui_layer) { // Don't call on_render for the UI layer here
                    layer->on_render(cmd);
                }
            }

            // 3. Render ImGui
            // The ImGuiLayer needs a separate render pass, or to be integrated
            // into the main one. For now, let's assume it does its own thing.
            m_imgui_layer->begin(); // Starts a new ImGui frame

            // Let all layers draw their UI components
            for (auto &layer: m_layer_stack) {
                layer->on_render_gui();
            }
            // Example: ImGui::ShowDemoWindow();

            m_imgui_layer->end(cmd); // Finishes ImGui frame and records draw commands to your command buffer

            cmd->end_render_pass();
            // 4. End the frame
            device->end_frame();
        }
    }

    /*void SandboxApp::on_render_gui() {
        //ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginMainMenuBar();

        // Render ImGui UI here
        for (auto &layer: m_layer_stack) {
            layer->on_render_gui();
        }

        ImGui::EndMainMenuBar();
        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float) m_window->get_width(),
                                (float) m_window->get_height());

        ImGui::Render();
        //this is now handled in the imgui pass

        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), nullptr);
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }*/

    void SandboxApp::on_event(Event &e) {
        // Handle events here

        ImGuiIO &io = ImGui::GetIO();
        e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

        if (!e.handled) {
            EventDispatcher dispatcher(e);
            dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &) {
                m_is_running = false;
                return true;
            });
            dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) {
                // Don't process if minimized
                if (e.get_width() == 0 || e.get_height() == 0) {
                    m_is_minimized = true;
                    return false;
                }
                m_is_minimized = false;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent &e) {
                auto key = e.get_key_code();
                if (e.is_repeat()) {
                    m_keyboard_state.keys_repeated[key] = true;
                    m_keyboard_state.keys_held_this_frame.insert(key);

                    // Check if the key is bound to an repeat action
                    if (m_key_repeat_bindings.find(key) != m_key_repeat_bindings.end()) {
                        m_key_repeat_bindings[key]();
                    }
                } else {
                    m_keyboard_state.keys_pressed_current_frame[key] = true;
                    m_keyboard_state.keys_held_this_frame.insert(key);

                    // Check if the key is bound to a single press action
                    if (m_key_press_bindings.find(key) != m_key_press_bindings.end()) {
                        m_key_press_bindings[key]();
                    }
                }
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<KeyReleasedEvent>([this](KeyReleasedEvent &e) {
                auto key = e.get_key_code();
                m_keyboard_state.keys_pressed_current_frame[key] = false;
                m_keyboard_state.keys_held_this_frame.erase(key);

                // Check if the key is bound to a release action
                if (m_key_release_bindings.find(key) != m_key_release_bindings.end()) {
                    m_key_release_bindings[key]();
                }
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e) {
                auto button = e.get_mouse_button();
                m_mouse_state.buttons_current_frame[button].is_pressed = true;
                m_mouse_state.buttons_current_frame[button].is_released = false;
                m_mouse_state.buttons_current_frame[button].button = button;
                m_mouse_state.buttons_current_frame[button].press_position =
                        m_mouse_state.cursor_position;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent &e) {
                auto button = e.get_mouse_button();
                m_mouse_state.buttons_current_frame[button].is_pressed = false;
                m_mouse_state.buttons_current_frame[button].is_released = true;
                m_mouse_state.buttons_current_frame[button].button = button;
                m_mouse_state.buttons_current_frame[button].release_position =
                        m_mouse_state.cursor_position;
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &e) {
                glm::vec2 last_position = m_mouse_state.cursor_position;
                m_mouse_state.cursor_position = glm::vec2(e.get_x(), e.get_y());
                m_mouse_state.delta_position =
                        m_mouse_state.cursor_position - last_position;
                m_mouse_state.is_moving =
                        glm::length(m_mouse_state.delta_position) > 0.0f;
                m_mouse_state.is_dragging = m_mouse_state.any_pressed();
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &e) {
                m_mouse_state.scroll_offset.x += e.get_x_offset();
                m_mouse_state.scroll_offset.y += e.get_y_offset();
                m_mouse_state.scroll_delta.x = e.get_x_offset();
                m_mouse_state.scroll_delta.y = e.get_y_offset();
                return false; // Allow layers to handle the event
            });
            dispatcher.dispatch<WindowFileDropEvent>([this](WindowFileDropEvent &e) {
                // Handle file drop event
                for (const auto &file_path: e.get_files()) {
                    m_asset_manager->force_load_from(file_path);
                    RDE_CORE_INFO("File dropped: {}", file_path);
                }
                return false; // Allow layers to handle the event
            });

            for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it) {
                if (e.handled) {
                    break;
                }
                (*it)->on_event(e);
            }
        }
    }
}



