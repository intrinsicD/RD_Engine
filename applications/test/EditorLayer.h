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

        void on_event(Event &) override {}

        void on_render(RAL::CommandBuffer *) override {}

        void on_render_gui() override;

        const char *get_name() const override { return "EditorLayer"; }

    private:
        void draw_entity_list();

        void draw_entity_inspector(entt::entity e);

        entt::registry &m_registry;
        SandboxApp *m_app = nullptr;
    };
}

