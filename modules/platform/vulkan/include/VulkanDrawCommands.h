#pragma once

#include "VulkanTypes.h"
#include <glm/mat4x4.hpp>

namespace RDE{
    struct DrawCommand {
        // --- State Objects ---
        // Handle to the complete pipeline state object (shaders, vertex layout, raster state, etc.)
        VkPipeline       pipeline;
        // Layout corresponding to the pipeline, needed for binding resources.
        VkPipelineLayout pipelineLayout;
        // Handle to the descriptor set for this object's material (textures, UBOs).
        VkDescriptorSet  descriptorSet;

        // --- Geometry Buffers ---
        // Handle to the vertex buffer for this object's mesh.
        VkBuffer         vertexBuffer;
        // Handle to the index buffer for this object's mesh.
        VkBuffer         indexBuffer;

        // --- Draw Parameters ---
        // The number of indices to draw from the index buffer.
        uint32_t         indexCount;
        // The per-object model matrix, typically sent via Push Constants.
        glm::mat4        modelMatrix;
    };

    struct InstancedDrawCommand {
        // --- Shared State Objects (for the whole batch) ---
        VkPipeline       pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSet  descriptorSet;

        // --- Shared Geometry Buffers (for the whole batch) ---
        VkBuffer         vertexBuffer; // The VBO of the mesh being instanced.
        VkBuffer         indexBuffer;  // The IBO of the mesh being instanced.
        uint32_t         indexCount;   // Index count for the single mesh.

        // --- Instancing Data ---
        // **NEW**: A buffer containing per-instance data (e.g., an array of model matrices).
        VkBuffer         instanceBuffer;
        // The number of instances to draw in this batch.
        uint32_t         instanceCount;
    };
}