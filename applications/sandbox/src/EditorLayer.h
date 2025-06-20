#pragma once

#include "Layer.h"
#include "Scene.h"
#include "Entity.h"

// ... other necessary includes like Scene, Entity
namespace RDE {
    class EditorLayer : public Layer {
    public:
        EditorLayer();

        void on_attach() override;

        void on_gui_render() override;

        void set_context(const std::shared_ptr<Scene> &scene);

    private:
        void draw_scene_hierarchy_panel();

        void draw_properties_panel();

        void register_component_uis();

        void save_scene(const std::string &filepath = "assets/scenes/MyScene.rde");

        void load_scene(const std::string &filepath = "assets/scenes/MyScene.rde");

        std::shared_ptr<Scene> m_scene;
        Entity m_selected_entity;
    };
}