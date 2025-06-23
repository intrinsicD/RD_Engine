// RDE_Project/modules/core/include/EntryPoint.h

#pragma once

#include "Application.h"
#include "Log.h"

// Forward declare the function the client must implement.
extern RDE::Application *RDE::CreateApplication();

// The main function is defined here, hidden away from the client.
// It initializes the core systems, creates the application specified by the client,
// runs it, and then cleans up.
// The engine's main entry point.
int main(int argc, char **argv) {
    RDE::Log::Initialize();
    RDE::RenderCommand::Init();

    auto app = RDE::CreateApplication();
    RDE_CORE_ASSERT(app, "Client application is null!");

    app->run();

    delete app; // This triggers all destructors in the correct order.

    return 0;
}