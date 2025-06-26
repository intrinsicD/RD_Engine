#pragma once

#include "ILayer.h"
#include "Scene.h"
#include "Entity.h"
#include "events/ApplicationEvent.h"

namespace RDE {
    class EditorLayer : public ILayer {
    public:
        explicit EditorLayer();

        void on_attach(const ApplicationContext &app_context, const FrameContext &frame_context) override;

        void on_gui_render(const ApplicationContext &app_context, const FrameContext &frame_context) override;

        void on_event(Event &e, const ApplicationContext &app_context, const FrameContext &frame_context);

    private:
        void draw_scene_hierarchy_panel(Scene *scene);

        void draw_properties_panel(Scene *scene);

        void register_component_uis(Scene *scene);

        void save_scene(const std::string &filepath = "assets/scenes/MyScene.rde");

        void create_renderable_entity_from_asset(const std::string &filepath = "assets/scenes/MyScene.rde", Scene *scene = nullptr, AssetManager *asset_manager = nullptr);

        Entity m_selected_entity;
    };
}