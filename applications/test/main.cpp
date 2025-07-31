#include "SandboxApp.h"
#include "src/GlfwVulkanWindow.h"
#include "core/EntryPoint.h"

namespace RDE {
    Application *CreateApplication() {
        //Here provide all dependencies to inject into the application
        WindowConfig config;
        config.title = "RDE SandboxApp";
        config.width = 1280;
        config.height = 720;
        auto window = GlfwVulkanWindow::Create(config);
        return new SandboxApp(std::move(window));
    }
}

int main() {
    RDE::Application *app = RDE::CreateApplication();
    try {
        app->run();
    } catch (const std::exception &e) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}