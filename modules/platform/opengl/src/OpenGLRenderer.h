#pragma once

#include "IRenderer.h"
#include <memory>

namespace RDE {

    // Forward declarations to keep this header clean
    class OpenGLDevice;

    class OpenGLCommandBuffer;

    class IWindow;

    class OpenGLRenderer : public IRenderer {
    public:
        explicit OpenGLRenderer(const RendererConfig &config);

        ~OpenGLRenderer() override;

        // --- IRenderer Interface Implementation ---
        bool init() override;

        void shutdown() override;

        void on_window_resize(int width, int height) override;

        void begin_frame() override;

        void submit(const RenderPacket &packet) override;

        void present_frame() override;

    private:
        RendererConfig m_config;
        IWindow *m_window; // Non-owning pointer, lifetime managed by Engine

        std::unique_ptr<OpenGLDevice> m_device;
        std::unique_ptr<OpenGLCommandBuffer> m_command_buffer;
    };

}