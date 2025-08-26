#pragma once

#include "core/ILayer.h"
#include <entt/entity/fwd.hpp>

namespace RDE {
    class SandboxApp; // forward

    class EditorLayer : public ILayer {
    public:
        EditorLayer(entt::registry &registry, SandboxApp *app) : m_registry(registry), m_app(app) {}
        ~EditorLayer() override = default;
        void on_attach() override {}
        void on_detach() override {}
        void on_update(float) override {}
        void on_event(Event &) override;
        void on_render(RAL::CommandBuffer *) override {}
        void on_render_gui() override;
        const char *get_name() const override { return "EditorLayer"; }
    private:
        void draw_entity_list();
        void draw_entity_inspector(entt::entity e);
        // Picking helpers
        std::string get_entity_label(entt::entity e, bool selected) const;
        bool pick_at_cursor(entt::entity &out_entity) const;
        entt::registry &m_registry;
        SandboxApp *m_app = nullptr;
    };
}
