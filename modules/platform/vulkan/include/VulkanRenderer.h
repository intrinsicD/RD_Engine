#pragma once

#include "IRenderer.h"
#include "VulkanDevice.h"
#include "VulkanTextureUploader.h" // Internal helper
#include <memory>

namespace RDE {

    // Concrete implementation of the IRenderer interface using Vulkan.
    class VulkanRenderer : public IRenderer {
    public:
    public:
        VulkanRenderer();

        ~VulkanRenderer() override;

        [[nodiscard]] FrameContext begin_frame() override;

        void submit_and_present(FrameContext &context) override;

        RAL::Device *get_device() override { return m_device.get(); }
    private:
        std::unique_ptr<VulkanDevice> m_device;
        uint32_t m_current_swapchain_image_index = 0; // Temp storage between begin/end
        bool m_frame_started = false; // Safety flag
    };

} // namespace RDE