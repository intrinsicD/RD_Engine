#pragma once

#include "AssetManager.h"

#include <variant>

namespace RDE::Components {
    using MaterialParameter = std::variant<float, int, glm::vec2, glm::vec3, glm::vec4, std::string>;

    struct MaterialComponent {
        MaterialParameter ambient;
        MaterialParameter diffuse;
        MaterialParameter specular;
        MaterialParameter emissive;
        MaterialParameter shininess;
        MaterialParameter alpha;

        AssetHandle program_handle;

        enum class PrimitiveTopology {
            Points,
            Lines,
            LineStrip,
            Triangles,
            TriangleStrip,
            TriangleFan
        } primitive_topology = PrimitiveTopology::Triangles;
    };
}
