// file: Renderer/Renderer.h (An example of the interface)
#pragma once

#include "AssetManager.h"
#include "EntityComponents/TransformComponent.h"
#include "EntityComponents/CameraProjectionComponent.h"
#include "EntityComponents/CameraComponent.h"
#include <vector>

namespace RDE {
    struct MeshAsset;
    struct MaterialAsset;

    class IRenderer {
    public:
        struct DrawCommand {
            MaterialAsset *material;
            MeshAsset *mesh;
            glm::mat4 transform; // Model matrix for the mesh
        };

        virtual ~IRenderer() = default;

        virtual bool compile_shader(ShaderAsset *shader_asset) = 0;

        virtual bool upload_mesh(MeshAsset *mesh_asset) = 0;

        virtual bool upload_texture(TextureAsset *texture_asset) = 0;

        virtual bool upload_camera(const CameraComponent *camera) = 0;

        virtual void bind_material(const MaterialAsset *material, AssetManager &asset_manager) = 0;

        // --- Scene Drawing Interface ---
        virtual void begin_scene(const TransformComponent &camera_transform,
                                 const CameraProjectionComponent &camera_projection,
                                 const CameraComponent &camera_cache) = 0;

        virtual void submit(const std::vector<DrawCommand> &commands) = 0;

        virtual void end_scene() = 0;
    };
}
