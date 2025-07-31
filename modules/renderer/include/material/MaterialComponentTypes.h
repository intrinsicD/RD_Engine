#pragma once

#include "MaterialDescription.h"

namespace RDE{
    struct MaterialSimplePBR : public MaterialDescription {
        Property<glm::vec3> base_color = parameters.add<glm::vec3>("base_color", glm::vec3(1.0f, 1.0f, 1.0f));
        Property<float> metallic = parameters.add<float>("metallic", 0.0f);
        Property<float> roughness = parameters.add<float>("roughness", 1.0f);
        Property<float> ao = parameters.add<float>("ao", 1.0f);
    };
}