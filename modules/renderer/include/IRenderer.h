#pragma once

#include "ral/Device.h" // The renderer needs to expose its device for certain setup.
#include "ral/CommandBuffer.h" // For resource processing.
#include "IWindow.h" // For resource processing.
#include "AssetDatabase.h" // For resource processing.

namespace RDE {
    struct FrameContext {
        bool is_valid = false;              // Is the frame renderable (e.g., not minimized)?
        RAL::CommandBuffer* command_buffer; // The primary command buffer to record into.
        RAL::TextureHandle backbuffer;      // Handle to the swapchain image for this frame.
        uint32_t frame_index;               // The current frame-in-flight index (0 or 1).
    };

    // Abstract interface for the entire rendering subsystem.
    // The Engine core holds a pointer to this and knows nothing about the implementation.
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // The application calls this at the start of its render phase.
        // It prepares the frame and provides the command buffer.
        [[nodiscard]] virtual FrameContext begin_frame() = 0;

        // The application calls this when it's done recording commands.
        // The renderer submits the work and presents the frame.
        virtual void submit_and_present(FrameContext& context) = 0;

        virtual RAL::Device* get_device() = 0;
    };

} // namespace RDE