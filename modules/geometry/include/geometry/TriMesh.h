#pragma once

#include "core/Properties.h"

namespace RDE{
    struct TriMesh {
        PropertyContainer vertices; // Vertices of the mesh
        PropertyContainer faces; // Optional, used for meshes

        size_t new_vertex() {
            vertices.push_back();
            return vertices.size() - 1; // Return the index of the new vertex
        }

        size_t new_face() {
            faces.push_back();
            return faces.size() - 1; // Return the index of the new face
        }
    };
}