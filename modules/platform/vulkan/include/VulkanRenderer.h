#pragma once

#include "IRenderer.h"
#include "VulkanDevice.h"
#include "VulkanTextureUploader.h" // Internal helper
#include <memory>

namespace RDE {

    // Concrete implementation of the IRenderer interface using Vulkan.
    class VulkanRenderer : public IRenderer {
    public:
        VulkanRenderer();

        ~VulkanRenderer() override;

        // --- Implementing the Interface ---
        void prepare_resources(AssetDatabase &asset_db) override;

        void draw_frame() override;

        RAL::Device *get_device() override;

        void shutdown() override;

    private:
        // The VulkanRenderer OWNS all the core Vulkan objects.
        std::unique_ptr<VulkanDevice> m_device;

        // It also owns its internal helper systems.
        std::unique_ptr<VulkanTextureUploader> m_texture_uploader;

        // Later, it will own these as well:
        // std::unique_ptr<RenderGraph> m_render_graph;
        // std::vector<std::unique_ptr<RAL::CommandBuffer>> m_command_buffers;
    };

} // namespace RDE