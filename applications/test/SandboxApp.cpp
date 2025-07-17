#include "SandboxApp.h"
#include "ImGuiLayer.h"
#include "TestSceneLayer.h"

#include "core/EntryPoint.h"
#include "core/events/ApplicationEvent.h"
#include "core/Log.h"

#include "core/Paths.h"
#include "core/Ticker.h"
#include "systems/TransformSystem.h"
#include "systems/CameraSystem.h"
#include "systems/HierarchySystem.h"
#include "systems/BoundingVolumeSystem.h"
#include "systems/RenderPacketSystem.h"

#include "assets/MeshLoader.h"
#include "assets/MaterialLoader.h"
#include "assets/StbImageLoader.h"
#include "assets/ShaderDefLoader.h"
#include "assets/PrefabLoader.h"
#include "assets/GenerateDefaultTextures.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace RDE {
    SandboxApp::SandboxApp(std::unique_ptr<IWindow> window) : m_window(
            std::move(window)) {
        Log::Initialize();
        m_window->set_event_callback(RDE_BIND_EVENT_FN(SandboxApp::on_event));
        m_renderer = std::make_unique<Renderer>(m_window.get());
        m_input_manager = std::make_unique<InputManager>();

        m_registry = std::make_shared<entt::registry>();
        m_dispatcher = std::make_shared<entt::dispatcher>();
        m_system_scheduler = std::make_unique<SystemScheduler>(*m_registry);
        GenerateDefaultTextures();
    }

    SandboxApp::~SandboxApp() {
        // Cleanup if necessary

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

        {
            m_asset_database = std::make_shared<AssetDatabase>();
            m_asset_manager = std::make_unique<AssetManager>(*m_asset_database);
            m_file_watcher = std::make_unique<FileWatcher>();
            m_file_watcher_event_queue = std::make_unique<ThreadSafeQueue<std::string> >();
            auto path = get_asset_path();

            m_file_watcher->start(path->string(), m_file_watcher_event_queue.get());
            //TODO: register loaders for different asset types
            m_asset_manager->register_loader(std::make_shared<MeshLoader>());
            m_asset_manager->register_loader(std::make_shared<StbImageLoader>());
            m_asset_manager->register_loader(std::make_shared<MaterialLoader>());
            m_asset_manager->register_loader(std::make_shared<ShaderDefLoader>());
            m_asset_manager->register_loader(std::make_shared<PrefabLoader>());
        }
        {
            m_system_scheduler->register_system<HierarchySystem>(*m_registry);
            m_system_scheduler->register_system<TransformSystem>(*m_registry);
            m_system_scheduler->register_system<BoundingVolumeSystem>(*m_registry);
            m_system_scheduler->register_system<CameraSystem>(*m_registry);
            m_system_scheduler->register_system<RenderPacketSystem>(*m_registry, *m_asset_database, m_main_view);
            RDE_INFO("Registered systems: HierarchySystem, TransformSystem, BoundingVolumeSystem, CameraSystem");
        }

        m_renderer->init();

        auto imgui_layer = std::make_shared<ImGuiLayer>(m_window.get(),
                                                        m_renderer->get_device());
        m_imgui_layer = imgui_layer.get();
        m_layer_stack.push_overlay(imgui_layer); // Assuming you have a push_layer method

        auto test_scene_layer = std::make_shared<TestSceneLayer>(m_asset_manager.get(), *m_registry);
        m_layer_stack.push_layer(test_scene_layer);
        return true;
    }

    void SandboxApp::shutdown() {
        m_layer_stack.clear();
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
        m_input_manager->process_held_actions(delta_time);

        while (!m_file_watcher_event_queue->empty()) {
            auto file_path_opt = m_file_watcher_event_queue->try_pop();
            if (file_path_opt) {
                const std::string &file_path = *file_path_opt;
                // Handle file change event, e.g., reload assets
                m_asset_manager->force_load(file_path);
            }
        }

        // Update layers
        for (auto &layer: m_layer_stack) {
            layer->on_update(delta_time);
        }

        m_system_scheduler->execute(delta_time);
    }

    void SandboxApp::on_render() {
        if (m_window_resized) {
            m_renderer->get_device()->recreate_swapchain();
            m_window_resized = false;
            return; // Skip this frame, we'll start fresh on the next one
        }

        if (RAL::CommandBuffer *cmd = m_renderer->begin_frame()) {

            // --- 1. Main Scene Render Pass ---
            // For now, we are just clearing the screen. In the future, you would
            // render your 3D scene here inside a render pass.
            RAL::RenderPassDescription scenePass{};
            scenePass.colorAttachments.resize(1);
            scenePass.colorAttachments[0].texture = RAL::TextureHandle::INVALID();
            scenePass.colorAttachments[0].loadOp = RAL::LoadOp::Clear; // CLEAR the screen
            scenePass.colorAttachments[0].storeOp = RAL::StoreOp::Store;
            scenePass.colorAttachments[0].clearColor[0] = 0.1f;
            scenePass.colorAttachments[0].clearColor[1] = 0.1f;
            scenePass.colorAttachments[0].clearColor[2] = 0.15f;
            scenePass.colorAttachments[0].clearColor[3] = 1.0f;
            cmd->begin_render_pass(scenePass);

            // 2. Let your scene layers render themselves (e.g., the 3D world)
            for (auto &layer: m_layer_stack) {
                if (layer.get() != m_imgui_layer) { // Don't call on_render for the UI layer here
                    layer->on_render(cmd);
                }
            }

            cmd->end_render_pass();

            // 3. Render ImGui
            m_imgui_layer->begin(); // Starts a new ImGui frame

            // Let all layers draw their UI components
            for (auto &layer: m_layer_stack) {
                layer->on_render_gui();
            }
            RAL::RenderPassDescription uiPass{};
            uiPass.colorAttachments.resize(1);
            uiPass.colorAttachments[0].texture = RAL::TextureHandle::INVALID();
            uiPass.colorAttachments[0].loadOp = RAL::LoadOp::Load; // LOAD the previous result
            uiPass.colorAttachments[0].storeOp = RAL::StoreOp::Store;
            cmd->begin_render_pass(uiPass); // Begin the UI pass

            m_imgui_layer->end(cmd); // Finishes ImGui frame and records draw commands to your command buffer

            cmd->end_render_pass();
            // 4. End the frame
            m_renderer->end_frame({cmd});
        } else {
            m_window_resized = true;
        }
        m_input_manager->on_frame_end();
    }

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
                m_window_resized = true;
                return false; // Allow layers to handle the event
            });
            m_input_manager->on_event(e);

            dispatcher.dispatch<WindowFileDropEvent>([this](WindowFileDropEvent &e) {
                // Handle file drop event
                for (const auto &file_path: e.get_files()) {
                    m_asset_manager->force_load(file_path);
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



