#pragma once

#include "core/Properties.h"
#include "HalfedgeMeshHandles.h"

namespace RDE {
    struct VertexConnectivity {
        HalfedgeHandle halfedge; // Halfedge that starts at this vertex
    };
    struct HalfedgeConnectivity {
        HalfedgeHandle next; // Next halfedge in the face
        HalfedgeHandle prev; // Previous halfedge in the face
        FaceHandle face; // Face this halfedge belongs to
        VertexHandle vertex; // Vertex this halfedge points to
    };
    struct FaceConnectivity {
        HalfedgeHandle halfedge; // Halfedge that starts the face
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

        PropertyContainer vertices;     // Vertices of the geometry
        PropertyContainer halfedges;    // Optional, used for half-edge structures
        PropertyContainer edges;        // Optional, used for edge structures
        PropertyContainer faces;        // Optional, used for polygonal meshes

        Property<VertexConnectivity> vconnectivity; // Vertex connectivity
        Property<HalfedgeConnectivity> hconnectivity; // Halfedge connectivity
        Property<FaceConnectivity> fconnectivity; // Face connectivity

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

        void set_halfedge(VertexHandle v, HalfedgeHandle h) {
            vconnectivity[v] = VertexConnectivity{h};
        }

        HalfedgeHandle get_halfedge(VertexHandle v) const {
            return vconnectivity[v].halfedge;
        }

        void set_next(HalfedgeHandle h, HalfedgeHandle next) {
            hconnectivity[h].next = next;
        }

        HalfedgeHandle get_next(HalfedgeHandle h) const {
            return hconnectivity[h].next;
        }

        void set_prev(HalfedgeHandle h, HalfedgeHandle prev) {
            hconnectivity[h].prev = prev;
        }

        HalfedgeHandle get_prev(HalfedgeHandle h) const {
            return hconnectivity[h].prev;
        }

        void set_face(HalfedgeHandle h, FaceHandle f) {
            hconnectivity[h].face = f;
        }

        FaceHandle get_face(HalfedgeHandle h) const {
            return hconnectivity[h].face;
        }

        void set_vertex(HalfedgeHandle h, VertexHandle v) {
            hconnectivity[h].vertex = v;
        }

        VertexHandle get_vertex(HalfedgeHandle h) const {
            return hconnectivity[h].vertex;
        }

        void set_halfedge(FaceHandle f, HalfedgeHandle h) {
            fconnectivity[f] = FaceConnectivity{h};
        }

        HalfedgeHandle get_halfedge(FaceHandle f) const {
            return fconnectivity[f].halfedge;
        }

        VertexHandle add_vertex(){
            return new_vertex();
        }

        //TODO continue here
        HalfedgeHandle add_halfedge(VertexHandle start, VertexHandle end) {
            HalfedgeHandle h = new_halfedge();
            set_halfedge(start, h);
            set_vertex(h, end);
            set_next(h, HalfedgeHandle::INVALID());
            set_prev(h, HalfedgeHandle::INVALID());
            set_face(h, FaceHandle::INVALID());
            return h;
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
        }
    };
}