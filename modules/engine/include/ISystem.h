#pragma once

namespace RDE {
    class Event;
    class Scene;
    class ApplicationContext;
    class FrameContext;

    // An interface for all systems that operate on the scene.
    // This provides a consistent structure for engine and application logic.
    class ISystem {
    public:
        virtual ~ISystem() = default;

        // --- SETUP & TEARDOWN HOOKS ---
        // Called once by the Scene when the system is first added.
        virtual void on_attach(const ApplicationContext &app_context, const FrameContext &frame_context) {
        }

        // Called once by the Scene just before it is destroyed.
        virtual void on_detach() {
        }


        // --- PHASE-BASED UPDATE HOOKS ---

        /**
         * @brief Called 0, 1, or N times per frame on a fixed timestep.
         * Ideal for physics, deterministic gameplay logic, and anything that
         * needs to be decoupled from the rendering frame rate for stability.
         * @param scene The scene to operate on.
         * @param services The global engine services.
         * @param fixed_timestep The fixed duration for this simulation step (e.g., 1/60th of a second).
         */
        virtual void on_update_simulation(const ApplicationContext &app_context, const FrameContext &frame_context) {
        }

        /**
         * @brief Called exactly once per rendered frame.
         * Ideal for logic that needs to run before rendering but is not
         * physics-critical, such as camera updates, animation interpolation,
         * or visual effects.
         * @param scene The scene to operate on.
         * @param services The global engine services.
         * @param frame_context The context for the current frame, containing delta_time.
         */
        virtual void on_update_presentation(const ApplicationContext &app_context, const FrameContext &frame_context) {
        }

    private:
        Scene *m_scene = nullptr; // Pointer to the scene this system operates on.
    };
}
