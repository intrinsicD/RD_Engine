#pragma once

#include "core/Properties.h"
#include "HalfedgeMeshHandles.h"

namespace RDE {
    struct VertexConnectivity {
        HalfedgeHandle halfedge = HalfedgeHandle::INVALID(); // Halfedge that starts at this vertex
    };

    struct HalfedgeConnectivity {
        HalfedgeHandle next = HalfedgeHandle::INVALID(); // Next halfedge in the face
        HalfedgeHandle prev = HalfedgeHandle::INVALID(); // Previous halfedge in the face
        FaceHandle face = FaceHandle::INVALID(); // Face this halfedge belongs to
        VertexHandle vertex = VertexHandle::INVALID(); // Vertex this halfedge points to
    };

    struct FaceConnectivity {
        HalfedgeHandle halfedge = HalfedgeHandle::INVALID(); // Halfedge that starts the face
    };

    class HalfedgeMesh {
    public:
        HalfedgeMesh() = default;

        HalfedgeMesh(const PropertyContainer &vertices,
                     const PropertyContainer &halfedges = PropertyContainer(),
                     const PropertyContainer &edges = PropertyContainer(),
                     const PropertyContainer &faces = PropertyContainer())
            : vertices(vertices), halfedges(halfedges), edges(edges), faces(faces) {
            init_properties();
        }

        HalfedgeMesh(PropertyContainer &vertices_,
                     PropertyContainer &halfedges_,
                     PropertyContainer &edges_,
                     PropertyContainer &faces_) {
            copy_ptrs(vertices_, halfedges_, edges_, faces_);
            init_properties();
        }

        PropertyContainer vertices; // Vertices of the geometry
        PropertyContainer halfedges; // Optional, used for half-edge structures
        PropertyContainer edges; // Optional, used for edge structures
        PropertyContainer faces; // Optional, used for polygonal meshes

        Property<VertexConnectivity> vconnectivity; // Vertex connectivity
        Property<HalfedgeConnectivity> hconnectivity; // Halfedge connectivity
        Property<FaceConnectivity> fconnectivity; // Face connectivity

        Property<bool> deleted_vertices;
        Property<bool> deleted_halfedges;
        Property<bool> deleted_edges;
        Property<bool> deleted_faces;

        void clear() {
            vertices.clear();
            halfedges.clear();
            edges.clear();
            faces.clear();
            init_properties();
        }

        VertexHandle new_vertex() {
            vertices.push_back();
            return static_cast<VertexHandle>(vertices.size() - 1);
        }

        HalfedgeHandle new_halfedge() {
            halfedges.push_back();
            return static_cast<HalfedgeHandle>(halfedges.size() - 1);
        }

        EdgeHandle new_edge() {
            edges.push_back();
            return static_cast<EdgeHandle>(edges.size() - 1);
        }

        FaceHandle new_face() {
            faces.push_back();
            return static_cast<FaceHandle>(faces.size() - 1);
        }

        void set_halfedge(const VertexHandle &v, const HalfedgeHandle &h) {
            vconnectivity[v] = VertexConnectivity{h};
        }

        HalfedgeHandle get_halfedge(const VertexHandle &v) const {
            return vconnectivity[v].halfedge;
        }

        void set_next(const HalfedgeHandle &h, const HalfedgeHandle &next) {
            hconnectivity[h].next = next;
        }

        HalfedgeHandle get_next(const HalfedgeHandle &h) const {
            return hconnectivity[h].next;
        }

        void set_prev(const HalfedgeHandle &h, const HalfedgeHandle &prev) {
            hconnectivity[h].prev = prev;
        }

        HalfedgeHandle get_prev(const HalfedgeHandle &h) const {
            return hconnectivity[h].prev;
        }

        void set_face(const HalfedgeHandle &h, const FaceHandle &f) {
            hconnectivity[h].face = f;
        }

        FaceHandle get_face(const HalfedgeHandle &h) const {
            return hconnectivity[h].face;
        }

        void set_vertex(const HalfedgeHandle &h, const VertexHandle &v) {
            hconnectivity[h].vertex = v;
        }

        VertexHandle get_vertex(const HalfedgeHandle &h) const {
            return hconnectivity[h].vertex;
        }

        void set_halfedge(const FaceHandle &f, const HalfedgeHandle &h) {
            fconnectivity[f] = FaceConnectivity{h};
        }

        HalfedgeHandle get_halfedge(const FaceHandle &f) const {
            return fconnectivity[f].halfedge;
        }

        HalfedgeHandle get_halfedge(const EdgeHandle &e, int index = 0) const {
            if (index < 0 || index > 1) {
                throw std::out_of_range("Index must be 0 or 1 for halfedge of an edge.");
            }
            return halfedges[e].halfedges[index];
        }

        VertexHandle add_vertex() {
            return new_vertex();
        }

        bool remove(const VertexHandle &v) {
            if (is_deleted(v)) {
                return false;
            }

            deleted_vertices[v] = true;
            return true;
        }

        bool remove(const HalfedgeHandle &h) {
            if (is_deleted(h)) {
                return false;
            }

            deleted_halfedges[h] = true;
            return true;
        }

        bool remove(const EdgeHandle &e) {
            if (is_deleted(e)) {
                return false;
            }

            deleted_edges[e] = true;
            return true;
        }

        bool remove(const FaceHandle &f) {
            if (is_deleted(f)) {
                return false;
            }

            deleted_faces[f] = true;
            return true;
        }


        bool is_valid(const VertexHandle &v) const { return vertices.size() > v.index && !deleted_vertices[v]; }

        bool is_valid(const HalfedgeHandle &h) const { return halfedges.size() > h.index && !deleted_halfedges[h]; }

        bool is_valid(const EdgeHandle &e) const { return edges.size() > e.index && !deleted_edges[e]; }

        bool is_valid(const FaceHandle &f) const { return faces.size() > f.index && !deleted_faces[f]; }

        bool is_deleted(const VertexHandle &v) const { return deleted_vertices[v]; }

        bool is_deleted(const HalfedgeHandle &h) const { return deleted_halfedges[h]; }

        bool is_deleted(const EdgeHandle &e) const { return deleted_edges[e]; }

        bool is_deleted(const FaceHandle &f) const { return deleted_faces[f]; }

        bool is_isolated(const VertexHandle &v) const {
            return !get_halfedge(v).is_valid() ;
        }

        bool is_boundary(const VertexHandle &v) const {
            HalfedgeHandle h = get_halfedge(v);
            return !h.is_valid() || !get_face(h).is_valid();
        }

        bool is_boundary(const HalfedgeHandle &h) const {
            return !get_face(h).is_valid();
        }

        bool is_boundary(const EdgeHandle &e) const {
            HalfedgeHandle h0 = get_halfedge(e, 0);
            HalfedgeHandle h1 = get_halfedge(e, 1);
            return is_boundary(h0) || is_boundary(h1);
        }

        bool is_boundary(const FaceHandle &f) const {
            //TODO how do i test the mesh face if its a boundary?
        }

        HalfedgeHandle get_opposite(const HalfedgeHandle &h) const {
            return HalfedgeHandle((h.index & 1) ? h.index - 1 : h.index + 1);
        }

        HalfedgeHandle rotate_cw(const HalfedgeHandle &h) const {
            return get_next(get_opposite(h));
        }

        HalfedgeHandle rotate_ccw(const HalfedgeHandle &h) const {
            return get_opposite(get_prev(h));
        }





        //TODO continue here
        HalfedgeHandle add_halfedge(const VertexHandle &start, const VertexHandle &end) {
            HalfedgeHandle h = find_halfedge(start, end);
            if (h.is_valid() && is_valid(h)) {
                return h; // Halfedge already exists
            }
            h = new_halfedge();
            set_halfedge(start, h);
            set_vertex(h, end);
            return h;
        }


        HalfedgeHandle add_edge(const VertexHandle &start, const VertexHandle &end) {
            HalfedgeHandle h = find_halfedge(start, end);
            if (h.is_valid() && is_valid(h)) {
                return h; // Edge already exists
            }
            h = new_halfedge();
            const HalfedgeHandle o = new_halfedge();

            const HalfedgeHandle hs = get_halfedge(start);
            const HalfedgeHandle he = get_halfedge(end);

            set_next(h, hs);
            set_prev(hs, h);
            set_next(o, he);
            set_prev(he, o);

            set_halfedge(start, h);
            set_vertex(h, end);

            set_halfedge(end, o);
            set_vertex(o, start);
            return h;
        }

        HalfedgeHandle find_halfedge(VertexHandle start, VertexHandle end) const {
            HalfedgeHandle h = get_halfedge(start);
            while (h.is_valid()) {
                if (get_vertex(h) == end) {
                    return h;
                }
                h = get_next(h);
            }
            return HalfedgeHandle::INVALID();
        }

        FaceHandle add_face(const std::vector<VertexHandle> &vertices) {
            if (vertices.empty()) {
                return FaceHandle::INVALID();
            }

            FaceHandle f = new_face();
            HalfedgeHandle first_halfedge = HalfedgeHandle::INVALID();
            HalfedgeHandle prev_halfedge = HalfedgeHandle::INVALID();

            for (const auto &v : vertices) {
                HalfedgeHandle h = add_edge(v, vertices[(vertices.size() + 1) % vertices.size()]);
                if (!first_halfedge.is_valid()) {
                    first_halfedge = h;
                }
                if (prev_halfedge.is_valid()) {
                    set_next(prev_halfedge, h);
                }
                set_prev(h, prev_halfedge);
                prev_halfedge = h;
            }

            set_next(prev_halfedge, first_halfedge);
            set_prev(first_halfedge, prev_halfedge);
            set_face(f, first_halfedge);

            return f;
        }

    private:
        void copy_ptrs(PropertyContainer &vertices_,
                       PropertyContainer &halfedges_,
                       PropertyContainer &edges_,
                       PropertyContainer &faces_) {
            vertices.copy_ptrs(vertices_);
            halfedges.copy_ptrs(halfedges_);
            edges.copy_ptrs(edges_);
            faces.copy_ptrs(faces_);
        }

        void init_properties() {
            vconnectivity = vertices.get_or_add<HalfedgeHandle>("v:connectivity");
            hconnectivity = halfedges.get_or_add<HalfedgeConnectivity>("h:connectivity");
            fconnectivity = faces.get_or_add<FaceConnectivity>("f:connectivity");
            deleted_vertices = vertices.get_or_add<bool>("deleted_vertices");
            deleted_halfedges = halfedges.get_or_add<bool>("deleted_halfedges");
            deleted_edges = edges.get_or_add<bool>("deleted_edges");
            deleted_faces = faces.get_or_add<bool>("deleted_faces");
        }
    };
}
