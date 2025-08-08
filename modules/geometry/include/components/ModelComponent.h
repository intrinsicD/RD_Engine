//geometry/components/ModelComponent.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace RDE {
    struct SubModelComponent {
        uint32_t index_offset{0}; // Offset in the geometry data
        uint32_t index_count{0}; // Number of indices in this sub-view
        uint32_t material_id{0};     // Material ID for this sub-view
        glm::vec3 aabb_min, aabb_max; // Axis-aligned bounding box for this sub-view
        glm::mat4 model{glm::mat4(1.0f)}; // Model matrix for this sub-view
        std::string name;            // Optional name for the sub-view
    };

    struct ModelComponent {
        std::vector<SubModelComponent> sub_models;
    };
}
