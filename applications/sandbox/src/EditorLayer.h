#pragma once

#include "ILayer.h"
#include "Scene.h"
#include "Entity.h"
#include "events/ApplicationEvent.h"

namespace RDE {
    class EditorLayer : public ILayer {
    public:
        explicit EditorLayer(Scene *scene);

        void on_attach() override;

        void on_gui_render() override;

        void on_event(Event &e);

    private:
        void draw_scene_hierarchy_panel();

        void draw_properties_panel();

        void register_component_uis();

        void save_scene(const std::string &filepath = "assets/scenes/MyScene.rde");

        void create_renderable_entity_from_asset(const std::string &filepath = "assets/scenes/MyScene.rde");

        Scene *m_scene;
        Entity m_selected_entity;
    };
}