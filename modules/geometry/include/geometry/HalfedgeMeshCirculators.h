#pragma once

namespace RDE {
    class HalfedgeMesh;

    class HalfedgeAroundVertexCirculator {
    public:
        // C++ standard iterator traits
        using iterator_category = std::forward_iterator_tag;
        using value_type = HalfedgeHandle;
        using difference_type = std::ptrdiff_t;
        using pointer = const HalfedgeHandle*;
        using reference = const HalfedgeHandle&;

        HalfedgeAroundVertexCirculator(const HalfedgeMesh* mesh, HalfedgeHandle start)
            : m_mesh(mesh), m_start(start), m_current(start) {}

        reference operator*() const { return m_current; }
        pointer operator->() const { return &m_current; }

        HalfedgeAroundVertexCirculator& operator++() {
            // Use the SAME rotation direction you want consistently. Let's use CCW.
            m_current = m_mesh->rotate_ccw(m_current);
            if (m_current == m_start) {
                // We've looped all the way around. Mark as completed.
                m_current = HalfedgeHandle::INVALID();
            }
            return *this;
        }

        // For range-based for loops, we only need operator!=
        friend bool operator!=(const HalfedgeAroundVertexCirculator& a, const HalfedgeAroundVertexCirculator& b) {
            return a.m_current != b.m_current;
        }

    private:
        const HalfedgeMesh* m_mesh;
        HalfedgeHandle m_start;
        HalfedgeHandle m_current;
    };

    class HalfedgeAroundFaceCirculator {
    public:
        HalfedgeAroundFaceCirculator(const FaceHandle &face, const HalfedgeMesh *mesh)
            : m_mesh(mesh) {
            if (m_mesh) {
                m_halfedge = m_mesh->get_halfedge(face);
                m_is_active = true;
            }
        }

        bool operator==(const HalfedgeAroundFaceCirculator &other) const {
            return m_is_active && m_halfedge == other.m_halfedge && m_mesh == other.m_mesh;
        }

        bool operator!=(const HalfedgeAroundFaceCirculator &other) const {
            return !operator==(other);
        }

        HalfedgeAroundFaceCirculator &operator++() {
            m_halfedge = m_mesh->get_next(m_halfedge);
            m_is_active = true;
            return *this;
        }

        HalfedgeAroundFaceCirculator operator++(int) {
            auto temp = *this;
            ++(*this);
            return temp;
        }

        HalfedgeAroundFaceCirculator &operator--() {
            m_halfedge = m_mesh->get_prev(m_halfedge);
            return *this;
        }

        HalfedgeAroundFaceCirculator &operator--(int) {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        const HalfedgeHandle &operator*() const { return m_halfedge; }

        operator bool() const { return m_halfedge.is_valid(); }

        HalfedgeAroundFaceCirculator &begin() {
            m_is_active = !m_halfedge.is_valid();
            return *this;
        }

        HalfedgeAroundFaceCirculator &end() {
            m_is_active = true;
            return *this;
        }

    private:
        HalfedgeHandle m_halfedge = HalfedgeHandle::INVALID(); // Current halfedge in the circulator
        const HalfedgeMesh *m_mesh = nullptr;
        bool m_is_active{false}; // helper for C++11 range-based for-loops
    };
}
