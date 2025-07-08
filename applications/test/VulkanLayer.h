#pragma once

#include "core/ILayer.h"
#include "ral/Device.h"
#include "ral/CommandBuffer.h"

namespace RDE {
    struct ApplicationContext; // Forward declaration of ApplicationContext

    class VulkanLayer : public ILayer {
    public:
        explicit VulkanLayer(ApplicationContext *app_context);

        ~VulkanLayer() override;

        void on_attach() override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_event(Event &event) override;

        const std::string &get_name() const override { return "VulkanLayer"; }

    private:
        ApplicationContext *m_app_context = nullptr; // Pointer to the application context
        std::unique_ptr<RAL::Device> m_device; // Pointer to the Vulkan device
        RAL::ShaderHandle vsHandle;
        RAL::ShaderHandle fsHandle;
        RAL::PipelineHandle pipelineHandle; // Handle to the Vulkan pipeline
        RAL::BufferHandle vbHandle, ibHandle; // Handle to the vertex buffer
    };
}