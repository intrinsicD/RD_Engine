#pragma once

#include "core/ILayer.h"
#include "assets/AssetManager.h"
#include "ral/Device.h"
#include "ral/Resources.h"
#include "renderer/Renderer.h"
#include "components/TransformComponent.h" // for TransformLocal

namespace RDE {
    // Minimal tag to mark triangle test entities
    struct TriangleTag {};

    class TestSceneLayer : public ILayer {
    public:
        TestSceneLayer(AssetManager *asset_manager, entt::registry &registry, RAL::Device *device, Renderer *renderer);

        ~TestSceneLayer() override;

        void on_attach() override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_event(Event &e) override;

        void on_render(RAL::CommandBuffer *cmd) override;

        void on_render_gui() override;

        [[nodiscard]] const char *get_name() const override {
            return "TestSceneLayer";
        }

    private:
        void create_triangle_resources();

        void destroy_triangle_resources();

        [[maybe_unused]] AssetManager *m_asset_manager = nullptr; // kept for future use (currently unused)
        entt::registry &m_registry;
        RAL::Device *m_device = nullptr;
        Renderer *m_renderer = nullptr; // to access camera descriptor set

        // Triangle GPU resources
        RAL::BufferHandle m_triangleVertexBuffer;
        RAL::PipelineHandle m_trianglePipeline;
        RAL::ShaderHandle m_triangleVS;
        RAL::ShaderHandle m_triangleFS;
    };
}