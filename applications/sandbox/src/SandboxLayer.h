#pragma once

#include "Layer.h"
#include "Scene.h"
#include "ISystem.h"

#include <memory>

namespace RDE {
    class RenderSystem;
    class CameraSystem;
    class InputSystem;

    class SandboxLayer : public Layer {
    public:
        SandboxLayer();

        void on_attach() override;

        void on_update(float delta_time) override;

        void on_event(Event &e) override;

        std::shared_ptr<Scene> get_scene() { return m_scene; }

    private:
        std::shared_ptr<Scene> m_scene;
        // List of systems that will be updated every frame
        //should i really do this?
        std::vector<std::unique_ptr<ISystem>> m_update_systems;
        std::vector<std::unique_ptr<ISystem>> m_render_systems;

        //for now keep these systems as pointers, later we can use a systems dependency graph
        //Should i really do this?
        RenderSystem *m_render_system = nullptr;
        CameraSystem *m_camera_system = nullptr;
        InputSystem *m_input_system = nullptr;
    };
}