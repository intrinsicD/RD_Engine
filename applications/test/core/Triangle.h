#pragma once

#include <glm/glm.hpp>

namespace RDE{
    struct Triangle{
        glm::vec3 a; // First vertex of the triangle
        glm::vec3 b; // Second vertex of the triangle
        glm::vec3 c; // Third vertex of the triangle

        glm::vec3 volume_vector() const {
            return glm::cross(b - a, c - a); // Volume vector is the cross product of two edges
        }

        // Function to compute the normal of the triangle
        glm::vec3 normal() const {
            return glm::normalize(volume_vector());
        }

        glm::vec3 centroid() const {
            return (a + b + c) / 3.0f; // Centroid is the average of the vertices
        }

        float area() const {
            return 0.5f * glm::length(volume_vector());
        }

        float perimeter() const {
            return glm::length(b - a) + glm::length(c - b) + glm::length(a - c); // Perimeter is the sum of edge lengths
        }
    };

    inline glm::vec3 ToBarycentricCoordinates(const Triangle &triangle, const glm::vec3 &point) {
        // Vectors from vertex A to the other vertices and to the point
        glm::vec3 v0 = triangle.b - triangle.a;
        glm::vec3 v1 = triangle.c - triangle.a;
        glm::vec3 v2 = point - triangle.a;

        // Compute dot products
        float d00 = glm::dot(v0, v0);
        float d01 = glm::dot(v0, v1);
        float d11 = glm::dot(v1, v1);
        float d20 = glm::dot(v2, v0);
        float d21 = glm::dot(v2, v1);

        // Compute barycentric coordinates
        float denom = d00 * d11 - d01 * d01;
        if (std::abs(denom) < 1e-6f) {
            // The triangle is degenerate (a line or a point).
            // Handle this case as you see fit, e.g., by returning an invalid coordinate.
            return glm::vec3(-1.0f);
        }

        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;

        return glm::vec3(u, v, w);
    }

    inline glm::vec3 FromBarycentricCoordinates(const Triangle &triangle, const glm::vec3 &bary) {
        return bary.x * triangle.a + bary.y * triangle.b + bary.z * triangle.c;
    }

    inline glm::vec3 ClosestPoint(const Triangle &triangle, const glm::vec3 &point) {
        //Robust implementation based on the method from the book "Real-Time Collision Detection" by Christer Ericson
        const glm::vec3& a = triangle.a;
        const glm::vec3& b = triangle.b;
        const glm::vec3& c = triangle.c;

        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 ap = point - a;

        // Check if P is in Voronoi region of vertex A
        const float d1 = glm::dot(ab, ap);
        const float d2 = glm::dot(ac, ap);
        if (d1 <= 0.0f && d2 <= 0.0f) {
            return a; // Closest to vertex A
        }

        // Check if P is in Voronoi region of vertex B
        const glm::vec3 bp = point - b;
        const float d3 = glm::dot(ab, bp);
        const float d4 = glm::dot(ac, bp);
        if (d3 >= 0.0f && d4 <= d3) {
            return b; // Closest to vertex B
        }

        // Check if P is in Voronoi region of edge AB
        const float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
            const float v = d1 / (d1 - d3);
            return a + v * ab; // Closest to edge AB
        }

        // Check if P is in Voronoi region of vertex C
        const glm::vec3 cp = point - c;
        const float d5 = glm::dot(ab, cp);
        const float d6 = glm::dot(ac, cp);
        if (d6 >= 0.0f && d5 <= d6) {
            return c; // Closest to vertex C
        }

        // Check if P is in Voronoi region of edge AC
        const float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
            const float w = d2 / (d2 - d6);
            return a + w * ac; // Closest to edge AC
        }

        // Check if P is in Voronoi region of edge BC
        const float va = d3 * d6 - d5 * d4;
        if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
            const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return b + w * (c - b); // Closest to edge BC
        }

        // P projects inside the face region of the triangle
        const float denom = 1.0f / (va + vb + vc);
        const float v = vb * denom;
        const float w = vc * denom;
        return a + ab * v + ac * w; // or u*a + v*b + w*c, where u = 1.0f - v - w
    }

    inline float SquaredDistance(const Triangle &triangle, const glm::vec3 &point) {
        const glm::vec3 closest = ClosestPoint(triangle, point);
        const glm::vec3 diff = closest - point;
        return glm::dot(diff, diff);
    }

    inline float Distance(const Triangle &triangle, const glm::vec3 &point) {
        return std::sqrt(SquaredDistance(triangle, point));
    }

    inline bool Contains(const Triangle &triangle, const glm::vec3 &point) {
        // A point is considered "contained" if it lies inside the triangle
        const glm::vec3 bary = ToBarycentricCoordinates(triangle, point);
        return bary.x >= 0.0f && bary.y >= 0.0f && bary.z >= 0.0f;
    }
}