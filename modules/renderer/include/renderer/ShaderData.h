#pragma once

#include <glm/glm.hpp>

// Corresponds to 'layout(set = 0, binding = 0) uniform CameraData'
struct alignas(16) CameraData {
    glm::mat4 view;
    glm::mat4 proj;
    // vec3 is aligned to 16 bytes in std140, so add padding.
    glm::vec3 camPos;
};

// Corresponds to 'layout(set =1, binding = 0) uniform MaterialData'
struct alignas(16) MaterialData {
    glm::vec4 baseColorFactor;
    float metalnessFactor;
    float roughnessFactor;
    // Add padding to meet std140 alignment rules if needed.
    float padding1;
    float padding2;
};