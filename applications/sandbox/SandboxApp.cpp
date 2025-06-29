// RDE_Project/applications/sandbox/SandboxApp.cpp

#include "Engine.h"
#include "../../modules/log/include/Log.h"
#include "src/GlfwOpenGLWindow.h"
#include "src/EditorLayer.h"
#include "src/SandboxLayer.h"
#include "OpenGLRenderer.h"
#include "JobSystem.h"
#include "assets/AssetManager.h"
#include "InputManager.h"
#include "EntryPoint.h"

namespace RDE {
    class SandboxApp {
    public:
        SandboxApp() {
            engine = CreateEngine();

            RDE_INFO("Sandbox application created!");
            // Push our main layer onto the stack.
            auto sandbox_layer  = engine->push_layer<SandboxLayer>();
            auto editor_layer = engine->push_layer<EditorLayer>();
        }

        ~SandboxApp() {
            RDE_INFO("Sandbox application destroyed!");
        }

        std::unique_ptr<Engine> engine;
    };

    // This is the function the engine's entry point will call.
    // We return a new instance of our Sandbox application.
    std::unique_ptr<Engine> CreateEngine() {
        WindowConfig window_config = {"SandboxApp", 1280, 720};

        auto window = std::make_unique<GlfwOpenGLWindow>(window_config);

        RendererConfig renderer_config = {
            .window = window.get(),
            .width = window_config.width,
            .height = window_config.height,
            .vsync = true,
        }; // Enable VSync
        auto renderer = std::make_unique<OpenGLRenderer>(renderer_config);
        auto job_system = std::make_unique<JobSystem>();
        auto asset_manager = std::make_unique<AssetManager>();
        auto input_manager = std::make_unique<InputManager>();

        // 2. Inject them into the Application.
        // The constructor will handle all initialization and wiring.
        return std::make_unique<Engine>(
            std::move(window),
            std::move(renderer),
            std::move(job_system),
            std::move(asset_manager),
            std::move(input_manager)
        );
    }
}
