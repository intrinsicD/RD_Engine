#pragma once

#include "IRenderer.h"
#include "glad/gl.h"

namespace RDE {
    class OpenGLRenderer : public IRenderer {
    public:
        OpenGLRenderer();

        virtual ~OpenGLRenderer() = default;

        bool compile_shader(ShaderAsset *shader_asset) override;

        bool upload_mesh(MeshAsset *mesh_asset) override;

        bool upload_texture(TextureAsset *texture_asset) override;

        bool upload_camera(const CameraComponent *camera) override;

        void bind_material(const MaterialAsset *material, AssetManager &asset_manager) override;

        // --- Scene Drawing Interface ---
        void begin_scene(const TransformComponent &camera_transform,
                         const CameraProjectionComponent &camera_projection,
                         const CameraComponent &camera) override;

        void submit(const std::vector<DrawCommand> &commands) override;

        void end_scene() override;
    private:
        GLuint m_camera_ubo = 0; // Uniform Buffer Object for camera data
    };
}
