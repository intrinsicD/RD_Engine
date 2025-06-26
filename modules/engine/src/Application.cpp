#include "Application.h"

#include "LayerStack.h"
#include "Base.h"
#include "layers/ImGuiLayer.h"
#include "assets/AssetManager.h"
#include "../../renderer/include/IRenderer.h"
#include "IWindow.h"
#include "JobSystem.h"
#include "Ticker.h"

namespace RDE {
    static Application *s_instance = nullptr;

    Application::Application(std::unique_ptr<IWindow> window, std::unique_ptr<IRenderer> renderer) : m_layer_stack(
            std::make_unique<LayerStack>()),
        m_window(std::move(window)),
        m_renderer(std::move(renderer)),
        m_asset_manager(std::make_unique<AssetManager>()){
        s_instance = this;
        m_window->set_event_callback(RDE_BIND_EVENT_FN(Application::on_event));

        auto imgui_layer = std::make_shared<ImGuiLayer>();
        m_imgui_layer = imgui_layer.get();
        push_overlay(imgui_layer);
        auto job_system = std::make_shared<JobSystem>();
        auto asset_manager = std::make_shared<AssetManager>();
        auto input_manager = std::make_shared<InputManager>();
        auto layers = std::make_shared<LayerStack>();
        m_engine = std::make_unique<Engine>(window, renderer, job_system, asset_manager, input_manager, layers);
    }

    Application::~Application() {
        m_layer_stack.reset();
        m_asset_manager.reset();
        m_renderer.reset();
        m_window.reset();
    }

    bool Application::on_initialize() {
        // This is where you would initialize your subsystems, load assets, etc.
        // Why not do this in the constructor?
        RDE_INFO("Application::on_initialize()\n");
        return true;
    }

    void Application::on_shutdown() {
        // This is where you would clean up resources, save settings, etc.
        // Why not cleanup in the destructor?
        RDE_CORE_INFO("Shutting down...");
    }

    void Application::run() {
        if (!on_initialize()) {
            // Creates Window, Renderer, JobSystem, AssetManager, Layers, etc.
            RDE_CORE_ERROR("Failed to initialize application");
            on_shutdown();
            return;
        }

        float time_accumulator = 0.0f;
        const float fixed_timestep = 1.0f / 60.0f; // For a 60Hz physics simulation

        Ticker ticker;
        while (m_is_running) {
            float delta_time = ticker.tick();
            time_accumulator += delta_time;

            m_renderer->begin_frame();
            //TODO here: m_job_system->begin_frame();
            for (auto &layer: *m_layer_stack) {
                layer->on_begin_frame(); // Layers can reset their own per-frame state
            }


            m_window->poll_events();

            while (time_accumulator >= fixed_timestep) {
                // This is the "tick" for physics and deterministic gameplay logic.
                for (auto &layer: *m_layer_stack) {
                    layer->on_fixed_update(fixed_timestep);
                }
                time_accumulator -= fixed_timestep;
            }

            for (const auto &layer: *m_layer_stack) {
                layer->on_variable_update(delta_time);
            }

            for (auto &layer: *m_layer_stack) {
                layer->on_render_submission();
            }

            m_imgui_layer->begin();
            for (const auto &layer: *m_layer_stack) {
                layer->on_gui_render();
            }
            m_imgui_layer->end();

            m_job_system->wait_for_all();
            m_renderer->execute_and_present();
        }
        on_shutdown();
    }

    void Application::on_event(Event &e) {
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

    ILayer *Application::push_layer(std::shared_ptr<ILayer> layer) {
        m_layer_stack->push_layer(layer);
        return layer.get();
    }

    ILayer *Application::push_overlay(std::shared_ptr<ILayer> overlay) {
        m_layer_stack->push_overlay(overlay);
        return overlay.get();
    }

    Application &Application::get() {
        return *s_instance;
    }
}
