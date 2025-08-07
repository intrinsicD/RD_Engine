#pragma once

#include <cstddef> // For size_t

namespace RDE{
    using IndexType = size_t;
    
    struct VertexHandle{
        IndexType index = -1; // Default to -1 for invalid handle

        operator IndexType() const {
            return index;
        }

        operator bool() const {
            return is_valid();
        }

        bool is_valid() const {
            return index != static_cast<IndexType>(-1);
        }
^
        bool operator==(const VertexHandle &other) const {
            return index == other.index;
        }

        bool operator!=(const VertexHandle &other) const {
            return index != other.index;
        }

        bool operator<(const VertexHandle &other) const {
            return index < other.index;
        }

        bool operator<=(const VertexHandle &other) const {
            return index <= other.index;
        }

        bool operator>(const VertexHandle &other) const {
            return index > other.index;
        }

        bool operator>=(const VertexHandle &other) const {
            return index >= other.index;
        }

        static VertexHandle INVALID() {
            return VertexHandle{static_cast<IndexType>(-1)};
        }
    };

    struct HalfedgeHandle{
        IndexType index;

        operator IndexType() const {
            return index;
        }

        operator bool() const {
            return is_valid();
        }

        bool is_valid() const {
            return index != static_cast<IndexType>(-1);
        }

        bool operator==(const HalfedgeHandle &other) const {
            return index == other.index;
        }

        bool operator!=(const HalfedgeHandle &other) const {
            return index != other.index;
        }

        bool operator<(const HalfedgeHandle &other) const {
            return index < other.index;
        }

        bool operator<=(const HalfedgeHandle &other) const {
            return index <= other.index;
        }

        bool operator>(const HalfedgeHandle &other) const {
            return index > other.index;
        }

        bool operator>=(const HalfedgeHandle &other) const {
            return index >= other.index;
        }

        static HalfedgeHandle INVALID() {
            return HalfedgeHandle{static_cast<IndexType>(-1)};
        }
    };
    struct EdgeHandle{
        IndexType index;

        operator IndexType() const {
            return index;
        }

        operator bool() const {
            return is_valid();
        }

        bool is_valid() const {
            return index != static_cast<IndexType>(-1);
        }

        bool operator==(const EdgeHandle &other) const {
            return index == other.index;
        }

        bool operator!=(const EdgeHandle &other) const {
            return index != other.index;
        }

        bool operator<(const EdgeHandle &other) const {
            return index < other.index;
        }

        bool operator<=(const EdgeHandle &other) const {
            return index <= other.index;
        }

        bool operator>(const EdgeHandle &other) const {
            return index > other.index;
        }

        bool operator>=(const EdgeHandle &other) const {
            return index >= other.index;
        }

        static EdgeHandle INVALID() {
            return EdgeHandle{static_cast<IndexType>(-1)};
        }
    };
    struct FaceHandle{
        IndexType index;

        operator IndexType() const {
            return index;
        }

        operator bool() const {
            return is_valid();
        }

        bool is_valid() const {
            return index != static_cast<IndexType>(-1);
        }

        bool operator==(const FaceHandle &other) const {
            return index == other.index;
        }

        bool operator!=(const FaceHandle &other) const {
            return index != other.index;
        }

        bool operator<(const FaceHandle &other) const {
            return index < other.index;
        }

        bool operator<=(const FaceHandle &other) const {
            return index <= other.index;
        }

        bool operator>(const FaceHandle &other) const {
            return index > other.index;
        }

        bool operator>=(const FaceHandle &other) const {
            return index >= other.index;
        }

        static FaceHandle INVALID() {
            return FaceHandle{static_cast<IndexType>(-1)};
        }
    };
}