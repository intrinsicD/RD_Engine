#pragma once

#include "ILayer.h"
#include "ISystem.h"
#include "Scene.h"
#include "AssetManager.h"
#include "events/ApplicationEvent.h"

#include <memory>
#include <vector>

namespace RDE {
    class SandboxLayer : public ILayer {
    public:
        SandboxLayer();

        void on_attach() override;

        void on_update(float delta_time) override;

        void on_event(Event &e) override;

        Scene *get_scene() const {
            return m_scene.get();
        }

    private:
        void on_window_file_drop(WindowFileDropEvent &e);

        // List of systems that will be updated every frame
        //should i really do this?
        std::unique_ptr<Scene> m_scene;
        std::vector<std::unique_ptr<ISystem> > m_systems;
    };
}
