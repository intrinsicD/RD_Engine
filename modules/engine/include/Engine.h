#pragma once

#include "LayerStack.h"
#include "Log.h"
#include "Scene.h"
#include "ISystem.h"
#include "IWindow.h"
#include "assets/AssetManager.h"
#include "JobSystem.h"
#include "Ticker.h"
#include "events/ApplicationEvent.h"

namespace RDE {
    class InputManager;

    class Engine {
    public:
        Engine(std::shared_ptr<IWindow> window,
               std::shared_ptr<IRenderer> renderer,
               std::shared_ptr<JobSystem> job_system,
               std::shared_ptr<AssetManager> asset_manager,
               std::shared_ptr<InputManager> input_manager,
               std::shared_ptr<LayerStack> layer_stack) :
                m_window(window),
                m_renderer(renderer),
                m_job_system(job_system),
                m_asset_manager(asset_manager),
                m_input_manager(input_manager),
                m_layer_stack(layer_stack) {
        }

        ~Engine() {
            on_shutdown();
        }

        bool on_initialize() {
            if (!m_window) {
                RDE_CORE_ERROR("Window is not initialized.");
                return false;
            } else {
                m_window->set_event_callback(RDE_BIND_EVENT_FN(Engine::on_event));
            }
            if (m_renderer && !m_renderer->init(m_window.get())) {
                RDE_CORE_ERROR("Renderer failed to initialize. Only compute is available.");
                return false;
            }
            if (m_job_system && !m_job_system->init()) {
                RDE_CORE_ERROR("JobSystem failed to initialize. Compute is now single-threaded.");
            }
            if (!m_asset_manager) {
                RDE_CORE_ERROR("AssetManager is not initialized.");
                return false;
            }
            if (!m_layer_stack) {
                RDE_CORE_ERROR("LayerStack is not initialized.");
                return false;
            }
            if (m_input_manager && !m_input_manager->init(m_window.get())) {
                RDE_CORE_ERROR("InputManager failed to initialize.");
                return false;
            }
        }

        void on_shutdown() {
            if (m_job_system) {
                RDE_CORE_INFO("Waiting for all jobs to finish before shutdown...");
                m_job_system->wait_for_all();
            }
            if (m_renderer) {
                RDE_CORE_INFO("Shutting down renderer...");
                m_renderer->shutdown();
            }
            if (m_layer_stack) {
                m_layer_stack.reset();
            }
            if (m_window) {
                RDE_CORE_INFO("Shutting down window...");
                m_window->close();
            }
            RDE_CORE_INFO("Engine shutdown complete.");
        }

        void run() {
            if (!on_initialize()) {
                RDE_CORE_ERROR("Failed to initialize engine");
                on_shutdown();
                return;
            }

            float time_accumulator = 0.0f;
            const float fixed_timestep = 1.0f / 60.0f; // For a 60Hz physics simulation

            // Main loop
            Ticker ticker;
            while (m_is_running) {
                float delta_time = ticker.tick();
                time_accumulator += delta_time;

                m_input_manager->process_events(); // Poll for input events, this will call the input manager's event handlers and afterwards set the input state for this frame.
                m_renderer->begin_frame(); // This is where you would begin the frame for the renderer, like clearing buffers, etc. Really? why not put that into a render pass?

                for (auto &layer: *m_layer_stack) {
                    layer->on_begin_frame(); // Layers can reset their own per-frame state
                }

                if (m_is_minimized) {
                    // Let background jobs run freely
                    time_accumulator = 0.0f;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                while (time_accumulator >= fixed_timestep) {
                    // This is the "tick" for physics and deterministic gameplay logic.
                    for (auto &layer: *m_layer_stack) {
                        layer->on_fixed_update(
                                fixed_timestep); // This is where you would update the physics and game logic
                    }
                    time_accumulator -= fixed_timestep;
                }

                for (const auto &layer: *m_layer_stack) {
                    layer->on_variable_update(
                            delta_time); // This is where you would update the game logic that is not physics-related, like animations, AI, etc.
                }

                for (auto &layer: *m_layer_stack) {
                    layer->on_render_submission(); // This is where you would submit the render commands to the renderer
                }

                m_job_system->wait_for_all(); // Wait for all jobs to finish before proceeding to rendering
                m_renderer->execute_render_commands(); // Execute the render commands that were submitted in the previous step
                m_renderer->present_frame(); // Present the rendered frame to the window
            }

            on_shutdown();
        }

        void set_scene(std::shared_ptr<Scene> scene) {
            m_scene = scene;
        }

        template<typename T>
        T *push_layer(std::shared_ptr<T> layer) {
            if (!m_layer_stack) {
                RDE_CORE_ERROR("Layer stack is not initialized.");
                return nullptr;
            }
            m_layer_stack->push_layer(layer);
            return layer.get();
        }

        template<typename T>
        T *push_overlay(std::shared_ptr<T> overlay) {
            if (!m_layer_stack) {
                RDE_CORE_ERROR("Layer stack is not initialized.");
                return nullptr;
            }
            m_layer_stack->push_overlay(overlay);
            return overlay.get();
        }
    public:
        std::shared_ptr<IWindow> m_window;
        std::shared_ptr<IRenderer> m_renderer;
        std::shared_ptr<JobSystem> m_job_system;
        std::shared_ptr<AssetManager> m_asset_manager;
        std::shared_ptr<InputManager> m_input_manager;

        std::shared_ptr<LayerStack> m_layer_stack;

        std::shared_ptr<Scene> m_scene;

        bool m_is_running = true;
        bool m_is_minimized = false;

    private:
        void on_event(Event &e) {
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

                m_renderer->on_window_resize(e.get_width(), e.get_height());
                // This function's only job is to manage the minimized state.
                // It MUST return false to allow layers to handle the event.
                return false;
            });

            for (auto it = m_layer_stack->rbegin(); it != m_layer_stack->rend(); ++it) {
                if (e.handled) {
                    break;
                }
                (*it)->on_event(e);
            }
        }
    };
}