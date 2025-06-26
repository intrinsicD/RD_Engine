#include "Engine.h"

#include "FrameContext.h"
#include "LayerStack.h"
#include "Log.h"
#include "Scene.h"
#include "IWindow.h"
#include "assets/AssetManager.h"
#include "JobSystem.h"
#include "InputManager.h"
#include "Ticker.h"
#include "events/ApplicationEvent.h"

#include <exception>

namespace RDE {
    Engine::Engine(std::unique_ptr<IWindow> window,
                   std::unique_ptr<IRenderer> renderer,
                   std::unique_ptr<JobSystem> job_system,
                   std::unique_ptr<AssetManager> asset_manager,
                   std::unique_ptr<InputManager> input_manager
    )
        : m_window(std::move(window)),
          m_renderer(std::move(renderer)),
          m_job_system(std::move(job_system)),
          m_asset_manager(std::move(asset_manager)),
          m_input_manager(std::move(input_manager)) {

        m_layer_stack = std::make_unique<LayerStack>();

        m_app_context = std::make_unique<ApplicationContext>();
        *m_app_context = {
            .window = m_window.get(),
            .renderer = m_renderer.get(),
            .job_system = m_job_system.get(),
            .asset_manager = m_asset_manager.get(),
            .input_manager = m_input_manager.get(),
            .layer_stack = m_layer_stack.get()
        };

        if (m_window && !m_window->init()) {
            m_window->close();
            throw std::runtime_error("Window initialization failed.");
        }
        if (!m_app_context) {
            throw std::runtime_error("Application initialization failed.");
        }

        m_window->set_event_callback(RDE_BIND_EVENT_FN(Engine::on_event));

        if (m_input_manager && !m_input_manager->init(*m_app_context)) {
            throw std::runtime_error("InputManager failed to initialize.");
        }
        if (m_renderer && !m_renderer->init(*m_app_context)) {
            throw std::runtime_error("Renderer failed to initialize.");
        }

        m_is_running = true;
        m_is_minimized = false;

        RDE_CORE_INFO("Engine initialized successfully.");
    }

    Engine::~Engine() {
        m_is_running = false;

        if (m_job_system) {
            RDE_CORE_INFO("Waiting for all jobs to finish...");
            m_job_system->wait_for_all();
        }
        if (m_layer_stack) {
            RDE_CORE_INFO("Detaching all layers...");
            m_layer_stack.reset();
        }
        if (m_renderer) {
            RDE_CORE_INFO("Shutting down renderer...");
            m_renderer->shutdown();
        }
        if (m_window) {
            RDE_CORE_INFO("Closing window...");
            m_window->close();
        }
        RDE_CORE_INFO("Engine shutdown complete.");
    }

    void Engine::run() {
        float time_accumulator = 0.0f;
        const float fixed_timestep = 1.0f / 60.0f; // For a 60Hz physics simulation

        // Main loop
        Ticker ticker;
        ApplicationContext &app_context = *m_app_context;
        while (m_is_running) {
            float delta_time = ticker.tick();

            FrameContext frame_context;
            frame_context.delta_time = delta_time;
            frame_context.fixed_time_step = fixed_timestep;
            frame_context.total_time += delta_time;
            frame_context.scene = m_scene.get();
            frame_context.is_minimized = m_is_minimized;

            m_input_manager->process_input();
            // Poll and process input events, this will call the input manager's event handlers and afterwards set the input state for this frame.
            for (auto &e: m_input_manager->fetch_events()) {
                on_event(e, app_context, frame_context);
            }

            if (m_is_minimized) {
                time_accumulator = 0.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            time_accumulator += delta_time;

            while (time_accumulator >= fixed_timestep) {
                // This is the "tick" for physics and deterministic gameplay logic.
                for (auto &layer: *m_layer_stack) {
                    layer->on_fixed_update(app_context, frame_context);
                }
                time_accumulator -= fixed_timestep;
            }

            for (const auto &layer: *m_layer_stack) {
                // This is where you would update the game logic that is not physics-related, like animations, AI, etc.
                layer->on_variable_update(app_context, frame_context);
            }

            m_renderer->begin_frame(); // Prepares for a new frame, gets swapchain image, etc.

            // 1. Build a list of things to render from the scene
            //    This can be done in parallel by a system.
            RenderPacket render_packet = m_scene->collect_renderables();

            // 2. Build the Render Graph
            //    The engine or renderer builds a graph based on the render packet.
            //    Layers can add their own passes (e.g., the EditorLayer adds a grid pass).
            RenderGraph render_graph("MainFrameGraph");
            setup_gbuffer_pass(render_graph, render_packet);
            setup_lighting_pass(render_graph, render_packet);
            for (auto& layer : *m_layer_stack) {
                layer->on_setup_render_graph(render_graph); // e.g., ImGui layer adds its UI pass
            }

            // 3. Compile and Execute the graph
            //    This step turns the high-level graph into a low-level command buffer.
            //    It optimizes, allocates resources, and records all commands.
            m_renderer->execute_graph(render_graph);

            // 4. Present
            m_renderer->present_frame();
        }
    }

    void Engine::on_event(Event &e, const ApplicationContext &app_context, const FrameContext &frame_context) {
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
            (*it)->on_event(e, app_context, frame_context);
        }
    }
}
