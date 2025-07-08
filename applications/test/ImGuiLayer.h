#pragma once

#include "core/ILayer.h"
#include "ral/Common.h"
#include "ral/Device.h"
#include "ral/CommandBuffer.h"

struct ImGuiContext;
struct ImDrawData;

namespace RDE {
    struct ApplicationContext; // Forward declaration of ApplicationContext

    class ImGuiLayer : public ILayer {
    public:
        explicit ImGuiLayer(ApplicationContext *app_context);

        ~ImGuiLayer() override;

        void on_attach() override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_event(Event &event) override;

        const std::string &get_name() const override { return "ImGuiLayer"; }

        //--------------------------------------------------------------------------------------------------------------
        void begin();

        void end(RAL::CommandBuffer &command_buffer);
        //--------------------------------------------------------------------------------------------------------------

    private:
        void setup_render_state(ImDrawData* draw_data, RAL::CommandBuffer* cmd, int fb_width, int fb_height);

        void create_ral_resources();

        void destroy_ral_resources();

        RAL::Device* m_Device = nullptr;
        ImGuiContext* m_Context = nullptr;

        // --- RAL Resources for ImGui Rendering ---
        RAL::PipelineHandle m_Pipeline;
        RAL::DescriptorSetLayoutHandle m_DsLayout; // Added
        RAL::DescriptorSetHandle m_DescriptorSet;  // Added

        RAL::TextureHandle m_FontTexture;
        RAL::SamplerHandle m_FontSampler;

        // We need to bind the font texture. This is our first encounter
        // with descriptor sets in the RAL. We'll define a simple version.
        // RAL::DescriptorSetHandle m_DescriptorSet;

        // Dynamic buffers that are recreated/updated each frame
        RAL::BufferHandle m_VertexBuffer;
        RAL::BufferHandle m_IndexBuffer;
        size_t m_VertexBufferSize = 0;
        size_t m_IndexBufferSize = 0;

        ApplicationContext *m_app_context = nullptr; // Pointer to the application context
    };
}