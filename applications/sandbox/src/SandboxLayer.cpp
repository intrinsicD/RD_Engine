#include "SandboxLayer.h"
#include "events/ApplicationEvent.h"
#include "Entity.h"

#include "systems/AnimationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CullingSystem.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

#include <filesystem>

namespace RDE {
    SandboxLayer::SandboxLayer() : ILayer("SandboxLayer") {
        m_scene = std::make_unique<Scene>();


    }

    void SandboxLayer::on_attach() {
        for (auto &system: m_systems) {
            system->on_attach(m_scene.get());
        }
    }

    void SandboxLayer::on_variable_update(float delta_time) {
        // --- 1. PRE-UPDATE PHASE ---
        // All systems prepare for the main update.
        for (auto &system: m_systems) {
            system->on_pre_update(m_scene.get(), delta_time);
        }

        // --- 2. MAIN UPDATE PHASE ---
        // The core logic of the simulation runs.
        for (auto &system: m_systems) {
            system->on_update(m_scene.get(), delta_time);
        }

        // --- 3. POST-UPDATE PHASE ---
        // All systems clean up their transient per-frame state.
        for (auto &system: m_systems) {
            system->on_post_update(m_scene.get(), delta_time);
        }
    }


    void SandboxLayer::on_event(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowFileDropEvent>(RDE_BIND_EVENT_FN(on_window_file_drop));
        for (auto &system: m_systems) {
            system->on_event(m_scene.get(), e);
        }
    }

    bool SandboxLayer::on_window_file_drop(WindowFileDropEvent &e) {
        // Get the asset manager from the scene context.

        for (const auto &path: e.get_files()) {
            std::filesystem::path fs_path(path);
            std::string extension = fs_path.extension().string();

            RDE_CORE_INFO("File dropped: {}, extension: {}", path, extension);
            //TODO call handler of this event
        }

        e.handled = true; // We've handled the event.
        return true; // Indicate that the event was handled.
    }
}
