#include "SandboxLayer.h"
#include "systems/CameraSystem.h"
#include "systems/RenderSystem.h"
#include "systems/InputSystem.h"

#include "../../../modules/assets/include/AssetManager.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "../../../modules/platform/opengl/src/OpenGLRenderer.h"

namespace RDE {
    SandboxLayer::SandboxLayer() : Layer("SandboxLayer") {
        m_scene = std::make_shared<Scene>();
        m_scene->get_registry().ctx().emplace<AssetManager>();
        m_update_systems.push_back(std::make_unique<InputSystem>());
        m_input_system = reinterpret_cast<InputSystem *>(m_update_systems.back().get());
        m_update_systems.push_back(std::make_unique<CameraSystem>());
        m_camera_system = dynamic_cast<CameraSystem *>(m_update_systems.back().get());

        m_render_systems.push_back(std::make_unique<RenderSystem>());
        m_render_system = reinterpret_cast<RenderSystem *>(m_render_systems.back().get());
        m_render_system->set_renderer(m_scene.get(), std::make_shared<OpenGLRenderer>());
    }

    void SandboxLayer::on_attach() {
        for (auto &system: m_update_systems) {
            system->on_attach(m_scene.get());
        }
    }

    void SandboxLayer::on_update(float delta_time) {
        // --- 1. PRE-UPDATE PHASE ---
        // All systems prepare for the main update.
        for (auto &system: m_update_systems) {
            system->on_pre_update(m_scene.get(), delta_time);
        }

        // --- 2. MAIN UPDATE PHASE ---
        // The core logic of the simulation runs.
        for (auto &system: m_update_systems) {
            system->on_update(m_scene.get(), delta_time);
        }

        // --- 3. POST-UPDATE PHASE ---
        // All systems clean up their transient per-frame state.
        for (auto &system: m_update_systems) {
            system->on_post_update(m_scene.get(), delta_time);
        }

        // --- 4. RENDERING ---
        // Rendering happens last, based on the final, settled state of the data.
        // The render systems would be called here.
        for (auto &system: m_render_systems) {
            system->on_pre_update(m_scene.get(), delta_time);
        }
        for (auto &system: m_render_systems) {
            system->on_update(m_scene.get(), delta_time);
        }
        for (auto &system: m_render_systems) {
            system->on_post_update(m_scene.get(), delta_time);
        }
    }

    void SandboxLayer::on_event(Event &e) {
        for (auto &system: m_update_systems) {
            system->on_event(m_scene.get(), e);
        }
    }
}
