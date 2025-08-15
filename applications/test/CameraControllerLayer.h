#pragma once

#include "core/ILayer.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "CameraControllers.h"
#include <entt/entity/entity.hpp>

namespace RDE {
    class Renderer;
    class IWindow;

    class CameraControllerLayer : public ILayer {
    public:
        CameraControllerLayer(entt::registry &registry, IWindow *window);
        ~CameraControllerLayer() override = default;

        void on_attach() override;
        void on_detach() override;
        void on_update(float delta_time) override;
        void on_event(Event &e) override;
        void on_render(RAL::CommandBuffer *) override {}
        void on_render_gui() override;
        const char *get_name() const override { return "CameraControllerLayer"; }
    private:
        bool capture_events() const; // returns true if ImGui wants mouse
        void sync_from_components();
        void sync_to_components();

        bool on_mouse_move(class MouseMovedEvent &e);
        bool on_mouse_button_pressed(class MouseButtonPressedEvent &e);
        bool on_mouse_button_released(class MouseButtonReleasedEvent &e);
        bool on_mouse_scrolled(class MouseScrolledEvent &e);

        entt::registry &m_registry;
        IWindow *m_window = nullptr;
        entt::entity m_camera_entity = entt::null;
        Camera::ViewParameters m_view_params{}; // controller-local
        Camera::ProjectionParameters *m_proj_params_ptr = nullptr; // non-owning
        std::unique_ptr<Camera::TrackballController> m_trackball;
        // Interaction state
        bool m_left_down = false;
        bool m_middle_down = false;
        glm::vec2 m_prev_mouse{0.0f};

        bool m_enable_input = true;
        bool m_ignore_imgui_capture = false;
        bool m_dirty_gui = false; // set when GUI edits camera
    };
}
