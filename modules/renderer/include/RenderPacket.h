#pragma once

#include "RendererTypes.h"

#include <glm/glm.hpp>
#include <vector>

namespace RDE {
    struct RenderObject {
        GpuGeometryHandle mesh_id; // Handle to a mesh resource TODO: make this a proper GpuHandle or GpuGeometryHandle...
        GpuMaterialHandle material_id; // Handle to a material resource
        glm::mat4 transform; // World transform matrix
    };

    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        float radius; // Attenuation radius
    };

    struct SpotLight {
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float radius; // Attenuation radius
        float inner_angle; // In radians
        float outer_angle; // In radians
    };

    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    };

    struct Frustum {
        glm::vec4 planes[6]; // 6 planes for the frustum
        glm::vec3 corners[8]; // 8 corners of the frustum
    };
    struct CameraData {
        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
        glm::vec3 position;
    };

    struct RenderPacket {
        // Main view information
        CameraData camera;
        Frustum camera_frustum; // Used for culling

        // Lists of visible objects, potentially sorted by material or state
        std::vector<RenderObject> opaque_objects;
        std::vector<RenderObject> transparent_objects;

        // Lights
        std::vector<PointLight> point_lights;
        std::vector<SpotLight> spot_lights;
        DirectionalLight sun_light;

        GpuTextureHandle skybox_texture;

        // ... other data like skybox info, reflection probes, etc.
    };
}
