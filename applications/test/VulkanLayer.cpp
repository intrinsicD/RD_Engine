#include "VulkanLayer.h"
#include "Application.h"
#include "Log.h"
#include "VulkanDevice.h" // Your concrete device
#include "ral/Resources.h"

#include <GLFW/glfw3.h>


namespace RDE {
    struct Vertex {
        float position[2];
        float color[3];
    };

    static const std::vector<Vertex> vertices = {
            {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
    };

    static const std::vector<uint16_t> indices = {
            0, 1, 2
    };

    VulkanLayer::VulkanLayer(ApplicationContext *app_context) : m_app_context(app_context) {
        RDE_CORE_ASSERT(m_app_context, "Application context cannot be null!");
        RDE_CORE_INFO("VulkanLayer created with ApplicationContext");
        glfwInit();

    }

    VulkanLayer::~VulkanLayer() {

    }

    void VulkanLayer::on_attach() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_app_context->m_window = glfwCreateWindow(800, 600, "Helios Engine", nullptr, nullptr);
        m_device = std::make_unique<RDE::VulkanDevice>(m_app_context->m_window);

        // 2. Create the Swapchain
        RAL::SwapchainDescription swapDesc{};
        swapDesc.nativeWindowHandle = m_app_context->m_window;
        swapDesc.vsync = true;
        m_device->create_swapchain(swapDesc);

        // 3. Create Shaders
        RAL::ShaderDescription vsDesc{};
        vsDesc.filePath = "shaders/spirv/vert.spv"; // Adjust path
        vsDesc.stage = RAL::ShaderStage::Vertex;
        vsHandle = m_device->create_shader(vsDesc);

        RAL::ShaderDescription fsDesc{};
        fsDesc.filePath = "shaders/spirv/frag.spv"; // Adjust path
        fsDesc.stage = RAL::ShaderStage::Fragment;
        fsHandle = m_device->create_shader(fsDesc);



        // 4. Create Buffers
        RAL::BufferDescription vbDesc{};
        vbDesc.size = sizeof(Vertex) * vertices.size();
        vbDesc.usage = RAL::BufferUsage::VertexBuffer; // You'll need to map this to the Vulkan flags
        vbDesc.memoryUsage = RAL::MemoryUsage::DeviceLocal; // And this
        vbDesc.initialData = vertices.data(); // This is where you would pass the initial
        vbHandle = m_device->create_buffer(
                vbDesc); // Note: Your create_buffer doesn't take initial data yet, you'd add that next

        RAL::BufferDescription ibDesc{};
        ibDesc.size = sizeof(uint16_t) * indices.size();
        ibDesc.usage = RAL::BufferUsage::IndexBuffer;
        ibDesc.memoryUsage = RAL::MemoryUsage::DeviceLocal;
        ibDesc.initialData = indices.data(); // This is where you would pass the initial
        ibHandle = m_device->create_buffer(ibDesc);

        // 5. Create the Graphics Pipeline
        RAL::PipelineDescription psoDesc{};
        psoDesc.vertexShader = vsHandle;
        psoDesc.fragmentShader = fsHandle;


        psoDesc.vertexBindings = {
                {.binding = 0, .stride = sizeof(Vertex)}
        };
        psoDesc.vertexAttributes = {
                {.location = 0, .binding = 0, .format = RAL::Format::R32G32_SFLOAT, .offset = offsetof(Vertex,
                                                                                                       position)},
                {.location = 1, .binding = 0, .format = RAL::Format::R32G32B32_SFLOAT, .offset = offsetof(Vertex,
                                                                                                          color)}
        };
        pipelineHandle = m_device->create_pipeline(psoDesc);
    }

    void VulkanLayer::on_detach() {
        m_device->wait_idle(); // Wait for GPU to finish before destroying resources

        m_device->destroy_pipeline(pipelineHandle);
        m_device->destroy_shader(vsHandle);
        m_device->destroy_shader(fsHandle);
        m_device->destroy_buffer(vbHandle);
        m_device->destroy_buffer(ibHandle);

        m_device.reset(); // Destroy the device, which also destroys swapchain etc.
    }

    void VulkanLayer::on_update(float delta_time) {
// This is where your simplified frame loop shines
        RAL::CommandBuffer *cmd = m_device->begin_frame();
        if (cmd) {
            RAL::RenderPassDescription passDesc{};
            // We need a way to get the swapchain texture. For now, let's assume
            // begin_render_pass can take a special handle or a null handle for the swapchain.
            // Or the command buffer could query it from the device.
            passDesc.colorAttachments.push_back({
                                                        //.texture = SWAPCHAIN_TEXTURE_HANDLE, // This needs to be solved
                                                        .loadOp = RAL::LoadOp::Clear,
                                                        .storeOp = RAL::StoreOp::Store,
                                                        .clearColor = {0.1f, 0.1f, 0.1f, 1.0f}
                                                });

            cmd->begin_render_pass(passDesc);

            // Set dynamic state
            cmd->set_viewport({0.f, 0.f, 800.f, 600.f, 0.f, 1.f});
            cmd->set_scissor({0, 0, 800, 600});

            // Bind resources and draw
            cmd->bind_pipeline(pipelineHandle);
            cmd->bind_vertex_buffer(vbHandle, 0);
            cmd->bind_index_buffer(ibHandle, RAL::IndexType::UINT16);
            cmd->draw_indexed(static_cast<uint32_t>(indices.size()));

            cmd->end_render_pass();

            m_device->end_frame();
        }
    }

    void VulkanLayer::on_event(Event &event) {

    }
}