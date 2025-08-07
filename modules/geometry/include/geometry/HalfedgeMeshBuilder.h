#pragma once
#include "HalfedgeMeshHandles.h"
#include "core/Properties.h"

namespace RDE {
    class HalfedgeMesh;

    template<typename PointType>
    class HalfedgeMeshBuilder {
    public:
        HalfedgeMeshBuilder(HalfedgeMesh &mesh) : m_mesh(mesh) {
            m_positions = m_mesh.vertices.get_or_add<PointType>("v:points", PointType());
        }

        VertexHandle add_vertex(const PointType &point) {
            auto v = m_mesh.vertices.new_vertex();
            if (v.is_valid()) {
                m_positions[v] = point;
            }
            return v;
        }

        HalfedgeHandle insert_vertex(const EdgeHandle &e, const PointType &point) {
            return insert_vertex(m_mesh.get_halfedge(e, 0), add_vertex(point));
        }

    protected:
        HalfedgeMesh &m_mesh;
        Property<PointType> m_positions;
    };
}
