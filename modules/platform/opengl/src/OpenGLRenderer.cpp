#include "OpenGLRenderer.h"
#include "OpenGLDevice.h"
#include "OpenGLDebug.h"
#include "OpenGLCommandBuffer.h"
#include "graph/RenderGraph.h"
#include "graph/RenderGraphBuilder.h"
#include "IWindow.h" // For swap_buffers

#include <iostream>

namespace RDE {

    // --- IRenderer Factory ---
    // This is how the engine will create an instance of our renderer.
    std::unique_ptr<IRenderer> IRenderer::Create(const RendererConfig& config) {
        return std::make_unique<OpenGLRenderer>(config);
    }

    // --- Constructor & Destructor ---
    OpenGLRenderer::OpenGLRenderer(const RendererConfig& config)
            : m_config(config), m_window(config.window) {}

    OpenGLRenderer::~OpenGLRenderer() {
        // m_device and m_command_buffer are cleaned up automatically by unique_ptr
    }

    // --- Lifecycle ---
    bool OpenGLRenderer::init() {
        if (!m_window) {
            RDE_CORE_ERROR("OpenGLRenderer::init error: Window is null!");
            return false;
        }

        // This assumes the windowing library (e.g. GLFW) is responsible for context creation.
        m_window->create_context();

        // Load OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)m_window->get_proc_address())) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        std::cout << "OpenGL Info:" << std::endl;
        std::cout << "  Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "  Version: " << glGetString(GL_VERSION) << std::endl;

        // Now that we have a context, create our device and command buffer
        m_device = std::make_unique<OpenGLDevice>();
        m_command_buffer = std::make_unique<OpenGLCommandBuffer>(m_device.get());

        // Set initial state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Good for skyboxes

        on_window_resize(m_config.width, m_config.height);

        return true;
    }

    void OpenGLRenderer::shutdown() {
        // Explicitly reset to control destruction order, though not strictly necessary
        m_command_buffer.reset();
        m_device.reset();
    }

    void OpenGLRenderer::on_window_resize(int width, int height) {
        m_config.width = width;
        m_config.height = height;
        // should i use here the m_command_buffer->set_viewport(0, 0, width, height);?
        glViewport(0, 0, width, height);
        GL_CHECK_ERROR();
    }

    // --- Frame Execution ---

    void OpenGLRenderer::begin_frame() {
        // Set the clear color and clear the default framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERROR();

        // Prepare the command buffer for a new frame's worth of commands
        m_command_buffer->begin();
    }

    void OpenGLRenderer::submit(const RenderPacket& packet) {
        RenderGraph rg;

        // --- Define a simple forward rendering pass ---
        // In a real PBR engine, this would be a G-Buffer pass, lighting pass, etc.
        rg.add_pass("Forward Pass",
                // 1. SETUP Lambda: Declares what this pass does with resources.
                // For this simple pass, we're just writing to the backbuffer, which is an
                // implicit dependency. We don't read or write any transient RG resources.
                    [&](RGBuilder& builder) {
                        // This pass has no transient resource dependencies.
                    },
                // 2. EXECUTE Lambda: The actual draw calls.
                    [&](ICommandBuffer& cmd, const RenderPacket& pkt) {
                        // Here, you would set up your camera UBO
                        // Example:
                        // m_device->update_buffer(m_camera_ubo, &pkt.camera, sizeof(CameraData));
                        // cmd.bind_uniform_buffer(m_camera_ubo, 0);

                        // Set viewport from the packet's camera/view
                        cmd.set_viewport(0, 0, m_config.width, m_config.height);

                        for (const auto& render_object : pkt.opaque_objects) {
                            // This is a simplified material system. We assume the pipeline is stored somewhere accessible.
                            // A real implementation would get the pipeline from the material.
                            // GpuPipelineHandle pipeline = m_device->get_material(render_object.material_id).pipeline;
                            // cmd.bind_pipeline(pipeline);

                            // Bind the geometry (which binds the VAO)
                            cmd.bind_vertex_buffer(m_device->get_geometry_desc(render_object.mesh_id).vertex_buffer);

                            // Update object-specific data (e.g., model matrix) via push constants or a UBO
                            // cmd.push_constants(ShaderStage::Vertex, &render_object.transform, sizeof(glm::mat4));

                            // Draw the object
                            cmd.draw_indexed(m_device->get_geometry_desc(render_object.mesh_id).index_count);
                        }
                    }
        );

        // Compile and execute the graph we just defined.
        rg.execute(*m_command_buffer, packet, m_device.get());
    }

    void OpenGLRenderer::present_frame() {
        // Mark the command buffer as finished for this frame
        m_command_buffer->end();

        // Tell the window to present the rendered image
        m_window->swap_buffers();
    }
}