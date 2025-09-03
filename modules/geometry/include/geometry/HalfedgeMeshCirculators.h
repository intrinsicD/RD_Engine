#pragma once

#include "HalfedgeMeshHandles.h"

#include <iterator>

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

        HalfedgeAroundVertexCirculator(const HalfedgeMesh* mesh, HalfedgeHandle start);

        reference operator*() const { return m_current; }
        pointer operator->() const { return &m_current; }

        HalfedgeAroundVertexCirculator& operator++();

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
        HalfedgeAroundFaceCirculator(const FaceHandle &face, const HalfedgeMesh *mesh);

        bool operator==(const HalfedgeAroundFaceCirculator &other) const {
            return m_is_active && m_halfedge == other.m_halfedge && m_mesh == other.m_mesh;
        }

        bool operator!=(const HalfedgeAroundFaceCirculator &other) const {
            return !operator==(other);
        }

        HalfedgeAroundFaceCirculator &operator++();

        HalfedgeAroundFaceCirculator operator++(int);

        HalfedgeAroundFaceCirculator &operator--();

        HalfedgeAroundFaceCirculator operator--(int);

        const HalfedgeHandle &operator*() const { return m_halfedge; }

        operator bool() const { return m_halfedge.is_valid(); }

        HalfedgeAroundFaceCirculator &begin();

        HalfedgeAroundFaceCirculator &end();

    private:
        HalfedgeHandle m_halfedge = HalfedgeHandle::INVALID(); // Current halfedge in the circulator
        const HalfedgeMesh *m_mesh = nullptr;
        bool m_is_active{false}; // helper for C++11 range-based for-loops
    };
}
