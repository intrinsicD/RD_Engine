#include "geometry/HalfedgeMeshCirculators.h"
#include "geometry/HalfedgeMesh.h"

namespace RDE {

    HalfedgeAroundVertexCirculator::HalfedgeAroundVertexCirculator(const HalfedgeMesh* mesh, HalfedgeHandle start)
        : m_mesh(mesh), m_start(start), m_current(start) {}

    HalfedgeAroundVertexCirculator& HalfedgeAroundVertexCirculator::operator++() {
        // Use the SAME rotation direction you want consistently. Let's use CCW.
        m_current = m_mesh->rotate_ccw(m_current);
        if (m_current == m_start) {
            // We've looped all the way around. Mark as completed.
            m_current = HalfedgeHandle::INVALID();
        }
        return *this;
    }

    HalfedgeAroundFaceCirculator::HalfedgeAroundFaceCirculator(const FaceHandle &face, const HalfedgeMesh *mesh)
            : m_mesh(mesh) {
        if (m_mesh) {
            m_halfedge = m_mesh->get_halfedge(face);
            m_is_active = true;
        }
    }

    HalfedgeAroundFaceCirculator &HalfedgeAroundFaceCirculator::operator++() {
        m_halfedge = m_mesh->get_next(m_halfedge);
        m_is_active = true;
        return *this;
    }

    HalfedgeAroundFaceCirculator HalfedgeAroundFaceCirculator::operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    HalfedgeAroundFaceCirculator &HalfedgeAroundFaceCirculator::operator--() {
        m_halfedge = m_mesh->get_prev(m_halfedge);
        return *this;
    }

    HalfedgeAroundFaceCirculator HalfedgeAroundFaceCirculator::operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    HalfedgeAroundFaceCirculator &HalfedgeAroundFaceCirculator::begin() {
        m_is_active = !m_halfedge.is_valid();
        return *this;
    }

    HalfedgeAroundFaceCirculator &HalfedgeAroundFaceCirculator::end() {
        m_is_active = true;
        return *this;
    }
}