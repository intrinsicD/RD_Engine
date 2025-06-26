#pragma once

#include "ApplicationContext.h"
#include "LayerStack.h"

#include <memory>

namespace RDE {
    class Scene;
    class Event;

    struct FrameContext;

    class Engine final {
    public:
        Engine(std::unique_ptr<IWindow> window,
                   std::unique_ptr<IRenderer> renderer,
                   std::unique_ptr<JobSystem> job_system,
                   std::unique_ptr<AssetManager> asset_manager,
                   std::unique_ptr<InputManager> input_manager);

        ~Engine();

        void run();

        /**
         * @brief Creates and pushes a new layer onto the layer stack.
         * The layer is constructed in-place with the provided arguments.
         * @tparam T The type of the layer to create (must derive from RDE::Layer).
         * @tparam Args The types of the arguments for the layer's constructor.
         * @return A raw pointer to the created layer.
         */
        template<typename T, typename... Args>
        T* push_layer(Args&&... args) {
            static_assert(std::is_base_of<ILayer, T>::value, "T must derive from ILayer");
            auto layer = std::make_shared<T>(std::forward<Args>(args)...);
            m_layer_stack->push_layer(layer);
            return layer.get();
        }

        /**
         * @brief Creates and pushes a new overlay onto the layer stack.
         * Overlays are always rendered after regular layers.
         */
        template<typename T, typename... Args>
        T* push_overlay(Args&&... args) {
            static_assert(std::is_base_of<ILayer, T>::value, "T must derive from ILayer");
            auto layer = std::make_shared<T>(std::forward<Args>(args)...);
            m_layer_stack->push_overlay(layer);
            return layer.get();
        }


        void set_active_scene(std::shared_ptr<Scene> scene) {
            m_scene = scene;
        }

        std::shared_ptr<Scene> get_scene() const { return m_scene; }

    private:
        void on_event(Event &e, const ApplicationContext &context, const FrameContext &frame_context);

        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IRenderer> m_renderer;
        std::unique_ptr<JobSystem> m_job_system;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<InputManager> m_input_manager;

        std::unique_ptr<LayerStack> m_layer_stack;

        std::shared_ptr<Scene> m_scene;

        bool m_is_running = true;
        bool m_is_minimized = false;

        std::unique_ptr<ApplicationContext> m_app_context;
    };

    std::unique_ptr<Engine> CreateEngine();
}
