#include "TestSceneLayer.h"
#include "core/Paths.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/HierarchyComponent.h"
#include "components/RenderableComponent.h"
#include "assets/AssetComponentTypes.h"
#include "core/FileIOUtils.h"
#include "core/Log.h"
#include "VulkanDevice.h" // NEW for swapchain extent

namespace RDE {
    TestSceneLayer::TestSceneLayer(AssetManager *asset_manager, entt::registry &registry, RAL::Device *device)
        : m_asset_manager(asset_manager), m_registry(registry), m_device(device) {}

    TestSceneLayer::~TestSceneLayer() {
        destroy_triangle_resources();
    }

    void TestSceneLayer::on_attach() {
        create_test_scene();
        create_triangle_resources();
    }

    void TestSceneLayer::on_detach() {
        destroy_triangle_resources();
    }

    void TestSceneLayer::on_update([[maybe_unused]] float delta_time) {}

    void TestSceneLayer::on_event([[maybe_unused]] Event &e) {}

    void TestSceneLayer::on_render([[maybe_unused]] RAL::CommandBuffer *cmd) {
        if(!cmd) return;
        if(!m_trianglePipeline.is_valid() || !m_triangleVertexBuffer.is_valid()) return;
        // Set dynamic viewport & scissor (required because pipeline uses dynamic states)
        if(auto *vkDev = dynamic_cast<VulkanDevice*>(m_device)) {
            auto extent = vkDev->get_swapchain().get_extent();
            RAL::Viewport vp{0.f,0.f,(float)extent.width,(float)extent.height,0.f,1.f};
            cmd->set_viewport(vp);
            RAL::Rect2D sc{0,0,extent.width,extent.height};
            cmd->set_scissor(sc);
        }
        cmd->bind_pipeline(m_trianglePipeline);
        cmd->bind_vertex_buffer(m_triangleVertexBuffer, 0);
        cmd->draw(3,1,0,0);
    }

    void TestSceneLayer::on_render_gui() {}

    void TestSceneLayer::create_test_scene() {
        // 1. Load/Create Assets (this is pseudocode, adapt to your AssetManager)
        // You need a simple material (shader, pipeline) and a mesh (cube, etc.)
        auto path = get_asset_path();
        if (!path.has_value()) {
            RDE_ERROR("Failed to get asset path");
            return;
        }

        auto f_cube_mesh_id = m_asset_manager->load_async(path.value() / "meshes" / "venus.obj");
        auto f_basic_material_id = m_asset_manager->load_async(path.value() / "materials" / "basic.mat");

        // 2. Create an entity
        auto entity = m_registry.create();

        // 3. Add components
        [[maybe_unused]] auto &local_transform = m_registry.emplace<TransformLocal>(entity);
        f_cube_mesh_id.wait();
        auto cube_mesh_id = f_cube_mesh_id.get();
        if(cube_mesh_id && cube_mesh_id->is_valid()){
            m_registry.emplace<RenderableComponent>(entity, cube_mesh_id); // Assuming get_id() returns AssetID
        }

        f_basic_material_id.wait();
        auto basic_material_id = f_basic_material_id.get();
        if(basic_material_id && basic_material_id->is_valid()) {
            m_registry.emplace<MaterialComponent>(entity, basic_material_id);
        }
        m_registry.emplace<Hierarchy>(entity); // If needed
    }

    void TestSceneLayer::create_triangle_resources() {
        if(!m_device) { RDE_CORE_WARN("TestSceneLayer: no device - cannot create triangle resources"); return; }
        if(m_trianglePipeline.is_valid()) return; // already created

        // 1. Create vertex buffer (pos(vec2) + color(vec3))
        struct Vertex { float px, py; float r,g,b; };
        Vertex vertices[3] = {
            { 0.0f,  -0.5f, 1.f, 0.f, 0.f},
            { 0.5f,   0.5f, 0.f, 1.f, 0.f},
            {-0.5f,   0.5f, 0.f, 0.f, 1.f}
        };
        RAL::BufferDescription vbDesc{}; vbDesc.size = sizeof(vertices); vbDesc.usage = RAL::BufferUsage::VertexBuffer; vbDesc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;
        m_triangleVertexBuffer = m_device->create_buffer(vbDesc);
        m_device->update_buffer_data(m_triangleVertexBuffer, vertices, sizeof(vertices), 0);

        // 2. Load precompiled SPIR-V shaders (assuming they exist at data/shaders/spirv/vert.spv & frag.spv)
        auto assetPath = get_asset_path();
        if(!assetPath) { RDE_CORE_ERROR("Asset path unavailable - triangle shaders missing"); return; }
        std::string vertSpv = (assetPath.value() / "shaders" / "spirv" / "vert.spv").string();
        std::string fragSpv = (assetPath.value() / "shaders" / "spirv" / "frag.spv").string();
        auto vertCode = FileIO::ReadFile(vertSpv);
        auto fragCode = FileIO::ReadFile(fragSpv);
        if(vertCode.empty() || fragCode.empty()) { RDE_CORE_ERROR("Failed to read triangle SPIR-V binaries"); return; }
        m_triangleVS = m_device->create_shader_module(vertCode, RAL::ShaderStage::Vertex);
        m_triangleFS = m_device->create_shader_module(fragCode, RAL::ShaderStage::Fragment);

        // 3. Create pipeline description (no descriptor sets, no push constants)
        RAL::PipelineDescription pDesc{};
        RAL::GraphicsShaderStages stages{}; stages.vertexShader = m_triangleVS; stages.fragmentShader = m_triangleFS; pDesc.stages = stages;
        // Vertex layout
        RAL::VertexInputBinding binding{}; binding.binding = 0; binding.stride = sizeof(Vertex); pDesc.vertexBindings.push_back(binding);
        RAL::VertexInputAttribute attrPos{}; attrPos.location = 0; attrPos.binding = 0; attrPos.format = RAL::Format::R32G32_SFLOAT; attrPos.offset = 0; attrPos.name = "POSITION"; pDesc.vertexAttributes.push_back(attrPos);
        RAL::VertexInputAttribute attrColor{}; attrColor.location = 1; attrColor.binding = 0; attrColor.format = RAL::Format::R32G32B32_SFLOAT; attrColor.offset = sizeof(float)*2; attrColor.name = "COLOR"; pDesc.vertexAttributes.push_back(attrColor);
        pDesc.topology = RAL::PrimitiveTopology::TriangleList;
        pDesc.rasterizationState.cullMode = RAL::CullMode::None; // show both sides
        // Match current render pass depth attachment format (present even if we do not test depth)
        pDesc.depthAttachmentFormat = RAL::Format::D32_SFLOAT; // NEW to satisfy validation
        // Explicitly keep depth test disabled
        pDesc.depthStencilState.depthTestEnable = false;
        pDesc.depthStencilState.depthWriteEnable = false;
        // Create pipeline
        m_trianglePipeline = m_device->create_pipeline(pDesc);
        if(!m_trianglePipeline.is_valid()) {
            RDE_CORE_ERROR("Failed to create triangle pipeline");
        }
    }

    void TestSceneLayer::destroy_triangle_resources() {
        if(!m_device) return;
        if(m_trianglePipeline.is_valid()) { m_device->destroy_pipeline(m_trianglePipeline); m_trianglePipeline = RAL::PipelineHandle::INVALID(); }
        if(m_triangleVS.is_valid()) { m_device->destroy_shader(m_triangleVS); m_triangleVS = RAL::ShaderHandle::INVALID(); }
        if(m_triangleFS.is_valid()) { m_device->destroy_shader(m_triangleFS); m_triangleFS = RAL::ShaderHandle::INVALID(); }
        if(m_triangleVertexBuffer.is_valid()) { m_device->destroy_buffer(m_triangleVertexBuffer); m_triangleVertexBuffer = RAL::BufferHandle::INVALID(); }
    }
}
