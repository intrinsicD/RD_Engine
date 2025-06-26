#include "systems/RenderSystem.h"
#include "Application.h"
#include "../../../renderer/include/IRenderer.h"
#include "Scene.h"
#include "Log.h"

#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "components/RenderableComponent.h"
#include "components/IsPrimaryTag.h"
#include "components/IsVisibleTag.h"

namespace RDE {
    void RenderSystem::on_attach(Scene *scene) {
        RDE_CORE_INFO("RenderSystem attached");
    }

    void RenderSystem::on_detach(Scene *scene) {
        RDE_CORE_INFO("RenderSystem detached");
    }

    CameraData GetPrimaryCameraData(Scene *scene) {
        auto &registry = scene->get_registry();
        auto view = registry.view<Components::CameraComponent, Components::IsPrimaryTag<
            Components::CameraComponent> >();

        if (view.size_hint() == 0) {
            return {
                .view = glm::mat4(1.0f),
                .projection = glm::mat4(1.0f)
            }; // Return a default camera data
        }

        // Assuming there's only one primary camera
        auto entity = view.front();
        auto &camera_component = registry.get<Components::CameraComponent>(entity);
        auto &transform = registry.get<Components::Transform>(entity);
        // Construct the camera data from the component
        CameraData camera_data;
        camera_data.view = glm::inverse(transform.model_matrix);
        camera_data.projection = camera_component.projection_matrix;
        return camera_data;
    }

    void RenderSystem::on_pre_update(Scene *scene, float delta_time) {
        // This system does not require pre-update logic
        // but can be used to prepare any state before the main update.

        Application &app = Application::get();
        auto &renderer = app.get_renderer();
        // Prepare the renderer for the frame
        //Collect all visible, renderable entities, prepare render queues, etc.

        {
            auto &registry = scene->get_registry();
            auto view = registry.view<Components::IsVisibleTag>();
            for (auto entity: view) {
                // Here you can collect all visible entities and prepare them for rendering
                // For example, you might want to gather their geometry and material handles
                // or any other data needed for rendering.
                // This is a good place to set up render queues or similar structures.
            }
        }
    }

    void RenderSystem::on_update(Scene *scene, float delta_time) {
        Application &app = Application::get();
        auto &renderer = app.get_renderer();

        // Begin the frame
        renderer.begin_frame();

        // Draw the frame with the camera data
        auto camera_data = GetPrimaryCameraData(scene);
        renderer.draw_frame(camera_data);

        // End the frame
        renderer.end_frame();
    }

    RenderPacket RenderSystem::collect_renderables(Scene *scene) {
        RenderPacket packet;

        // Collect camera data
        auto camera_data = GetPrimaryCameraData(scene);
        packet.view_matrix = camera_data.view;
        packet.projection_matrix = camera_data.projection;
        packet.camera_position = glm::vec3(camera_data.view[3]);

        // Collect all visible renderable objects
        auto &registry = scene->get_registry();
        auto view = registry.view<RenderObject, Components::Transform, Components::IsVisibleTag>();

        for (auto entity: view) {
            auto &renderable = registry.get<RenderObject>(entity);
            auto &transform = registry.get<Components::Transform>(entity);

            renderable.transform = transform.model_matrix;
            RenderObject &obj = renderable;
            packet.opaque_objects.push_back(obj);
        }

        return std::move(packet);
    }

    struct GBufferHandles {
        RGResourceHandle albedo;
        RGResourceHandle normals;
        RGResourceHandle depth;
    };

    GBufferHandles setup_gbuffer_pass(RenderGraph &rg, const RenderPacket &packet) {
        GBufferHandles handles;

        rg.add_pass("G-Buffer Pass",
                    // 1. SETUP Lambda: Declares I/O using the builder. Runs immediately.
                    [&](RGBuilder &builder) {
                        // Create virtual textures for our G-Buffer
                        handles.albedo = builder.create_texture({.format = TextureFormat::RGBA8, ...});
                        handles.normals = builder.create_texture({.format = TextureFormat::RGBA16F, ...});
                        handles.depth = builder.create_texture({.format = TextureFormat::D32F, ...});

                        // Declare that this pass writes to these textures.
                        builder.write(handles.albedo);
                        builder.write(handles.normals);
                        builder.write(handles.depth);

                        // This pass doesn't read any RG resources, but it does read the global scene geometry
                        // which is handled implicitly via the RenderPacket.
                    },
                    // 2. EXECUTE Lambda: The actual draw calls. Runs later during graph execution.
                    [=](ICommandBuffer &cmd, const RenderPacket &pkt) {
                        // The renderer has already bound the render targets and set the viewport for us.
                        // cmd.bind_pipeline(gbuffer_pipeline);
                        for (const auto &object: pkt.objects) {
                            // cmd.bind_material(object.material);
                            // cmd.push_constants(object.transform);
                            // cmd.draw_geometry(object.geometry);
                        }
                    }
        );
        return handles;
    }

    void setup_lighting_pass(RenderGraph &rg, const RenderPacket &packet, const GBufferHandles &gbuffer) {
        rg.add_pass("Deferred Lighting Pass",
                    // 1. SETUP
                    [&](RGBuilder &builder) {
                        // This pass READS the G-Buffer textures. This creates the dependency edge.
                        builder.read(gbuffer.albedo);
                        builder.read(gbuffer.normals);
                        builder.read(gbuffer.depth);

                        // It WRITES to the final backbuffer/swapchain image (we'd have a handle for this).
                        // RGResourceHandle backbuffer_handle = get_backbuffer_handle();
                        // builder.write(backbuffer_handle);
                    },
                    // 2. EXECUTE
                    [=](ICommandBuffer &cmd, const RenderPacket &pkt) {
                        // The renderer has bound the G-Buffer textures as shader inputs for us.
                        // cmd.bind_pipeline(lighting_pipeline);
                        // Draw a full-screen quad to run the lighting shader.
                        // cmd.draw_fullscreen_quad();
                    }
        );
    }
}
