#pragma once

#include "RenderPacket.h"

#include <memory>

namespace RDE {
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual bool init() = 0;

        virtual void shutdown() = 0;

        virtual void on_window_resize(unsigned int width, unsigned int height) = 0;

        virtual void begin_frame() = 0; // Still useful for starting frame logic, getting swapchain image

        virtual void submit(const RenderPacket& packet) = 0; // This is the main function called by the application loop

        virtual void present_frame() = 0;

        static std::unique_ptr<IRenderer> Create(const RendererConfig &config = RendererConfig());
    };
}


