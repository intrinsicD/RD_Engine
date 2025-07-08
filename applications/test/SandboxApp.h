#pragma once

#include "core/Application.h"
#include "core/IWindow.h"

namespace RDE {
    class SandboxApp : public Application {
    public:
        SandboxApp(std::unique_ptr<IWindow> window = nullptr);

        ~SandboxApp() override;

        void run(int width, int height, const char *title) override;

    private:
        bool init(int width, int height, const char *title) override;

        void shutdown() override;

        void on_update(float delta_time) override;

        void on_render() override;

        void on_render_gui() override;

        void on_event(Event &e) override;

        ApplicationContext &get_app_context() {
            return *m_app_context;
        }

        std::unique_ptr<IWindow> m_window;
        std::shared_ptr<ApplicationContext> m_app_context; // Application context containing the main window and state
    };
}
