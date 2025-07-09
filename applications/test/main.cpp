#include "SandboxApp.h"
#include "src/GlfwVulkanWindow.h"
#include "core/EntryPoint.h"

namespace RDE {
    Application *CreateApplication() {
        //Here provide all dependencies to inject into the application
        auto window = GlfwVulkanWindow::Create();
        return new SandboxApp(std::move(window));
    }
}

int main() {
    RDE::Application *app = RDE::CreateApplication();
    try {
        app->run(1280, 720, "RDE SandboxApp");
    } catch (const std::exception &e) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}