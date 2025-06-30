#pragma once

#include "ral/Device.h" // The renderer needs to expose its device for certain setup.
#include "AssetDatabase.h" // For resource processing.

namespace RDE {

    // Abstract interface for the entire rendering subsystem.
    // The Engine core holds a pointer to this and knows nothing about the implementation.
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // A high-level function that encapsulates all per-frame setup that needs
        // access to the asset database (e.g., texture uploads, mesh processing).
        virtual void prepare_resources(AssetDatabase& asset_db) = 0;

        // The main drawing function. This will eventually take scene data.
        virtual void draw_frame() = 0;

        // Provides access to the underlying abstract device.
        // This is needed for engine-level setup, like creating the swapchain.
        virtual RAL::Device* get_device() = 0;

        virtual void shutdown() = 0;
    };

} // namespace RDE