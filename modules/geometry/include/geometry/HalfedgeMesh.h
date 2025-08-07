#pragma once

#include "core/Properties.h"
#include "core/Log.h"
#include "HalfedgeMeshHandles.h"
#include "HalfedgeMeshCirculators.h"

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

        void free_memory() {
            vertices.free_memory();
            halfedges.free_memory();
            edges.free_memory();
            faces.free_memory();
        }

        void garbage_collection() {
            if (!m_has_garbage)
                return;

            auto nV = vertices.size();
            auto nE = edges.size();
            auto nH = halfedges.size();
            auto nF = faces.size();

            // setup handle mapping
            auto vmap = vertices.add<VertexHandle>("v:garbage-collection");
            auto hmap = halfedges.add<HalfedgeHandle>("h:garbage-collection");
            auto fmap = faces.add<FaceHandle>("f:garbage-collection");

            for (size_t i = 0; i < nV; ++i) { vmap[VertexHandle(i)] = VertexHandle(i); }
            for (size_t i = 0; i < nH; ++i) { hmap[HalfedgeHandle(i)] = HalfedgeHandle(i); }
            for (size_t i = 0; i < nF; ++i) { fmap[FaceHandle(i)] = FaceHandle(i); }

            // remove deleted vertices
            if (nV > 0) {
                size_t i0 = 0;
                size_t i1 = nV - 1;

                while (true) {
                    // find first deleted and last un-deleted
                    while (!deleted_vertices[VertexHandle(i0)] && i0 < i1) { ++i0; }
                    while (deleted_vertices[VertexHandle(i1)] && i0 < i1) { --i1; }
                    if (i0 >= i1) { break; }

                    // swap
                    vertices.swap(i0, i1);
                }

                // remember new size
                nV = deleted_vertices[VertexHandle(i0)] ? i0 : i0 + 1;
            }

            // remove deleted edges
            if (nE > 0) {
                size_t i0 = 0;
                size_t i1 = nE - 1;

                while (true) {
                    // find first deleted and last un-deleted
                    while (!deleted_edges[EdgeHandle(i0)] && i0 < i1) { ++i0; }
                    while (deleted_edges[EdgeHandle(i1)] && i0 < i1) { --i1; }
                    if (i0 >= i1) { break; }

                    // swap
                    edges.swap(i0, i1);
                    halfedges.swap(2 * i0, 2 * i1);
                    halfedges.swap(2 * i0 + 1, 2 * i1 + 1);
                }

                // remember new size
                nE = deleted_edges[EdgeHandle(i0)] ? i0 : i0 + 1;
                nH = 2 * nE;
            }

            // remove deleted faces
            if (nF > 0) {
                size_t i0 = 0;
                size_t i1 = nF - 1;

                while (true) {
                    // find 1st deleted and last un-deleted
                    while (!deleted_faces[FaceHandle(i0)] && i0 < i1) { ++i0; }
                    while (deleted_faces[FaceHandle(i1)] && i0 < i1) { --i1; }
                    if (i0 >= i1) { break; }

                    // swap
                    faces.swap(i0, i1);
                }

                // remember new size
                nF = deleted_faces[FaceHandle(i0)] ? i0 : i0 + 1;
            }

            // update vertex connectivity
            for (size_t i = 0; i < nV; ++i) {
                auto v = VertexHandle(i);
                if (!is_isolated(v)) { set_halfedge(v, hmap[get_halfedge(v)]); }
            }

            // update halfedge connectivity
            for (size_t i = 0; i < nH; ++i) {
                auto h = HalfedgeHandle(i);
                set_vertex(h, vmap[get_vertex(h)]);
                set_next(h, hmap[get_next(h)]);
                if (!is_boundary(h)) { set_face(h, fmap[get_face(h)]); }
            }

            // update handles of faces
            for (size_t i = 0; i < nF; ++i) {
                auto f = FaceHandle(i);
                set_halfedge(f, hmap[get_halfedge(f)]);
            }

            // remove handle maps
            vertices.remove(vmap);
            halfedges.remove(hmap);
            faces.remove(fmap);

            // finally resize arrays
            vertices.resize(nV);
            vertices.free_memory();
            halfedges.resize(nH);
            halfedges.free_memory();
            edges.resize(nE);
            edges.free_memory();
            faces.resize(nF);
            faces.free_memory();

            m_num_deleted_vertices = m_num_deleted_edges = m_num_deleted_faces = 0;
            m_has_garbage = false;
        }

        VertexHandle new_vertex() {
            vertices.push_back();
            return static_cast<VertexHandle>(vertices.size() - 1);
        }

        FaceHandle add_face(const std::vector<VertexHandle> &vertices) {
            return add_face(vertices, false, false);
        }

        FaceHandle add_triangle(const VertexHandle &v0, const VertexHandle &v1, const VertexHandle &v2) {
            return add_face({v0, v1, v2});
        }

        FaceHandle add_quad(const VertexHandle &v0, const VertexHandle &v1, const VertexHandle &v2,
                            const VertexHandle &v3) {
            return add_face({v0, v1, v2, v3});
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

        FaceHandle get_face(const EdgeHandle &e, const int i = 0) const {
            if (i < 0 || i > 1) {
                return FaceHandle::INVALID();
            }
            return FaceHandle{(e.index << 1) + i};
        }

        void set_vertex(const HalfedgeHandle &h, const VertexHandle &v) {
            hconnectivity[h].vertex = v;
        }

        VertexHandle get_vertex(const HalfedgeHandle &h) const {
            return hconnectivity[h].vertex;
        }

        VertexHandle get_vertex(const EdgeHandle &e, const int i = 0) const {
            if (i < 0 || i > 1) {
                return VertexHandle::INVALID();
            }
            return VertexHandle{(e.index << 1) + i};
        }

        void set_halfedge(const FaceHandle &f, const HalfedgeHandle &h) {
            fconnectivity[f] = FaceConnectivity{h};
        }

        HalfedgeHandle get_halfedge(const FaceHandle &f) const {
            return fconnectivity[f].halfedge;
        }

        HalfedgeHandle get_halfedge(const EdgeHandle &e, const int i = 0) const {
            if (i < 0 || i > 1) {
                return HalfedgeHandle::INVALID();
            }
            return HalfedgeHandle{(e.index << 1) + i};
        }

        bool mark_deleted(const VertexHandle &v) {
            if (is_deleted(v)) {
                return false;
            }

            deleted_vertices[v] = true;
            return true;
        }

        bool mark_deleted(const HalfedgeHandle &h) {
            if (is_deleted(h)) {
                return false;
            }

            deleted_halfedges[h] = true;
            return true;
        }

        bool mark_deleted(const EdgeHandle &e) {
            if (is_deleted(e)) {
                return false;
            }

            deleted_edges[e] = true;
            return true;
        }

        bool mark_deleted(const FaceHandle &f) {
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
            return !get_halfedge(v).is_valid();
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
            for (const auto &h: get_halfedges(f)) {
                if (is_boundary(h)) {
                    return true;
                }
            }
            return false;
        }

        bool is_manifold(const VertexHandle &v) const {
            // A vertex is manifold if it has at most one outgoing boundary halfedge
            int boundary_count = 0;
            for (const auto &h: get_halfedges(v)) {
                if (is_boundary(h)) {
                    ++boundary_count;
                    if (boundary_count > 1) {
                        return false; // More than one boundary halfedge found
                    }
                }
            }
            return true; // At most one boundary halfedge found
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

        HalfedgeAroundVertexCirculator get_halfedges(const VertexHandle &v) const {
            HalfedgeHandle start_he = get_halfedge(v);
            return HalfedgeAroundVertexCirculator(this, start_he);
        }

        HalfedgeAroundFaceCirculator get_halfedges(const FaceHandle &f) const {
            return HalfedgeAroundFaceCirculator(f, this);
        }

        HalfedgeHandle find_halfedge(const VertexHandle &start, const VertexHandle &end) const {
            for (const auto &h: get_halfedges(start)) {
                if (get_vertex(h) == end) {
                    return h;
                }
            }
            return HalfedgeHandle::INVALID();
        }

        void adjust_outgoing_halfedge(const VertexHandle &v) {
            for (const auto &h: get_halfedges(v)) {
                if (is_boundary(h)) {
                    set_halfedge(v, h);
                    return; // Found a boundary halfedge
                }
            }
        }

        HalfedgeHandle insert_vertex(const EdgeHandle &e, const VertexHandle &v) {
            return insert_vertex(get_halfedge(e, 0), v);
        }

        HalfedgeHandle insert_vertex(const HalfedgeHandle &h0, const VertexHandle &v) {
            // before:
            //
            // v0      h0       v2
            //  o--------------->o
            //   <---------------
            //         o0
            //
            // after:
            //
            // v0  h0   v   h1   v2
            //  o------>o------->o
            //   <------ <-------
            //     o0       o1

            HalfedgeHandle h2 = get_next(h0);
            HalfedgeHandle o0 = get_opposite(h0);
            HalfedgeHandle o2 = get_prev(o0);
            VertexHandle v2 = get_vertex(h0);
            FaceHandle fh = get_face(h0);
            FaceHandle fo = get_face(o0);

            HalfedgeHandle h1 = new_edge(v, v2);
            HalfedgeHandle o1 = get_opposite(h1);

            // adjust halfedge connectivity
            set_next(h1, h2);
            set_next(h0, h1);
            set_vertex(h0, v);
            set_vertex(h1, v2);
            set_face(h1, fh);

            set_next(o1, o0);
            set_next(o2, o1);
            set_vertex(o1, v);
            set_face(o1, fo);

            // adjust vertex connectivity
            set_halfedge(v2, o1);
            adjust_outgoing_halfedge(v2);
            set_halfedge(v, h1);
            adjust_outgoing_halfedge(v);

            // adjust face connectivity
            if (fh.is_valid()) { set_halfedge(fh, h0); }
            if (fo.is_valid()) { set_halfedge(fo, o1); }

            return o1;
        }

        bool is_collapse_ok(const HalfedgeHandle &v0v1) const {
            HalfedgeHandle v1v0 = get_opposite(v0v1);
            VertexHandle v0 = get_vertex(v1v0);
            VertexHandle v1 = get_vertex(v0v1);
            VertexHandle vl, vr;
            HalfedgeHandle h1, h2;

            // the edges v1-vl and vl-v0 must not be both boundary edges
            if (!is_boundary(v0v1)) {
                vl = get_vertex(get_next(v0v1));
                h1 = get_next(v0v1);
                h2 = get_next(h1);
                if (is_boundary(get_opposite(h1)) &&
                    is_boundary(get_opposite(h2)))
                    return false;
            }

            // the edges v0-vr and vr-v1 must not be both boundary edges
            if (!is_boundary(v1v0)) {
                vr = get_vertex(get_next(v1v0));
                h1 = get_next(v1v0);
                h2 = get_next(h1);
                if (is_boundary(get_opposite(h1)) && is_boundary(get_opposite(h2))) { return false; }
            }

            // if vl and vr are equal or both invalid -> fail
            if (vl == vr) { return false; }

            // edge between two boundary vertices should be a boundary edge
            if (is_boundary(v0) && is_boundary(v1) && !is_boundary(v0v1) && !is_boundary(v1v0)) { return false; }

            // test intersection of the one-rings of v0 and v1
            for (auto vhv: get_halfedges(v0)) {
                auto vv = get_vertex(vhv);
                if (vv != v1 && vv != vl && vv != vr) {
                    if (find_halfedge(vv, v1).is_valid()) { return false; }
                }
            }

            // passed all tests
            return true;
        }

        //TODO continue here with removal ok?

    protected:
        HalfedgeHandle new_halfedge() {
            halfedges.push_back();
            return static_cast<HalfedgeHandle>(halfedges.size() - 1);
        }

        HalfedgeHandle new_halfedge(const VertexHandle &start, const VertexHandle &end) {
            if (start == end) {
                return HalfedgeHandle::INVALID(); // Cannot create a halfedge from a vertex to itself
            }
            HalfedgeHandle h = new_halfedge();
            set_vertex(h, start);
            return h;
        }

        EdgeHandle new_edge() {
            edges.push_back();
            return static_cast<EdgeHandle>(edges.size() - 1);
        }

        HalfedgeHandle new_edge(const VertexHandle &start, const VertexHandle &end) {
            if (start == end) {
                return HalfedgeHandle::INVALID();
            }
            new_edge();
            HalfedgeHandle h = new_halfedge(start, end);
            new_halfedge(end, start);
            return h;
        }

        FaceHandle new_face() {
            faces.push_back();
            return static_cast<FaceHandle>(faces.size() - 1);
        }

        HalfedgeHandle add_edge(const VertexHandle &start, const VertexHandle &end) {
            HalfedgeHandle h = find_halfedge(start, end);
            if (h.is_valid() && is_valid(h)) {
                return h; // Edge already exists
            }
            new_edge(start, end);
            h = new_halfedge();
            const HalfedgeHandle o = new_halfedge();

            set_vertex(h, end);
            set_vertex(o, start);
            return h;
        }

        FaceHandle add_face(const std::vector<VertexHandle> &vertices, bool allow_complex_vertex,
                            bool allow_complex_edge) {
            if (vertices.empty()) {
                return FaceHandle::INVALID();
            }

            const size_t N = vertices.size();
            if (N < 3) {
                return FaceHandle::INVALID();
            }

            std::vector<HalfedgeHandle> &add_face_halfedges = m_add_face_halfedges;
            std::vector<bool> &add_face_is_new = m_add_face_is_new;
            std::vector<bool> &add_face_needs_adjust = m_add_face_needs_adjust;
            NextCache &add_face_next_cache = m_add_face_next_cache;
            add_face_halfedges.clear();
            add_face_halfedges.resize(N);
            add_face_is_new.clear();
            add_face_is_new.resize(N);
            add_face_needs_adjust.clear();
            add_face_needs_adjust.resize(N, false);
            add_face_next_cache.clear();
            add_face_next_cache.reserve(3 * N);

            // test for topological errors
            for (size_t i = 0, ii = 1; i < N; ++i, ++ii, ii %= N) {
                if (!is_boundary(vertices[i])) {
                    if (!allow_complex_vertex) {
                        RDE_CORE_ERROR("SurfaceMesh::add_face: Complex vertex.");
                        return FaceHandle::INVALID();
                    } else {
                        RDE_CORE_WARN("SurfaceMesh::add_face: Complex vertex.");
                    }
                }

                add_face_halfedges[i] = find_halfedge(vertices[i], vertices[ii]);
                add_face_is_new[i] = !add_face_halfedges[i].is_valid();

                if (!add_face_is_new[i] && !is_boundary(add_face_halfedges[i])) {
                    if (!allow_complex_edge) {
                        RDE_CORE_ERROR("SurfaceMesh::add_face: Complex edge.");
                        return FaceHandle::INVALID();
                    } else {
                        RDE_CORE_WARN("SurfaceMesh::add_face: Complex edge.");
                    }
                }
            }

            // re-link patches if necessary
            for (size_t i = 0, ii = 1; i < N; ++i, ++ii, ii %= N) {
                if (!add_face_is_new[i] && !add_face_is_new[ii]) {
                    HalfedgeHandle inner_prev = add_face_halfedges[i];
                    HalfedgeHandle inner_next = add_face_halfedges[ii];

                    if (get_next(inner_prev) != inner_next) {
                        // here comes the ugly part... we have to relink a whole patch

                        // search a free gap
                        // free gap will be between boundaryPrev and boundaryNext
                        HalfedgeHandle outer_prev = get_opposite(inner_next);
                        HalfedgeHandle outer_next = get_opposite(inner_prev);
                        HalfedgeHandle boundary_prev = outer_prev;
                        do {
                            boundary_prev = get_opposite(get_next(boundary_prev));
                        } while (!is_boundary(boundary_prev) ||
                                 boundary_prev == inner_prev);
                        HalfedgeHandle boundary_next = get_next(boundary_prev);
                        assert(is_boundary(boundary_prev));
                        assert(is_boundary(boundary_next));

                        // ok ?
                        if (boundary_next == inner_next) {
                            RDE_CORE_ERROR("SurfaceMesh::add_face: Patch re-linking failed.");
                            return FaceHandle::INVALID();
                        }

                        // other halfedges' handles
                        HalfedgeHandle patch_start = get_next(inner_prev);
                        HalfedgeHandle patch_end = get_prev(inner_next);

                        // relink
                        add_face_next_cache.emplace_back(boundary_prev, patch_start);
                        add_face_next_cache.emplace_back(patch_end, boundary_next);
                        add_face_next_cache.emplace_back(inner_prev, inner_next);
                    }
                }
            }


            // create missing edges
            for (size_t i = 0, ii = 1; i < N; ++i, ++ii, ii %= N) {
                if (add_face_is_new[i]) {
                    add_face_halfedges[i] = new_edge(vertices[i], vertices[ii]);
                }
            }

            // create the face
            FaceHandle f = new_face();
            set_halfedge(f, add_face_halfedges[N - 1]);

            // setup halfedges
            for (size_t i = 0, ii = 1; i < N; ++i, ++ii, ii %= N) {
                VertexHandle v = vertices[ii];
                HalfedgeHandle inner_prev = add_face_halfedges[i];
                HalfedgeHandle inner_next = add_face_halfedges[ii];

                size_t id = 0;
                if (add_face_is_new[i]) {
                    id |= 1;
                }
                if (add_face_is_new[ii]) {
                    id |= 2;
                }

                if (id) {
                    HalfedgeHandle outer_prev = get_opposite(inner_next);
                    HalfedgeHandle outer_next = get_opposite(inner_prev);

                    // set outer links
                    switch (id) {
                        case 1: // prev is new, next is old
                            HalfedgeHandle boundary_prev = get_prev(inner_next);
                            add_face_next_cache.emplace_back(boundary_prev, outer_next);
                            set_halfedge(v, outer_next);
                            break;

                        case 2: // next is new, prev is old
                            HalfedgeHandle boundary_next = get_next(inner_prev);
                            add_face_next_cache.emplace_back(outer_prev, boundary_next);
                            set_halfedge(v, boundary_next);
                            break;

                        case 3: // both are new
                            if (!get_halfedge(v).is_valid()) {
                                set_halfedge(v, outer_next);
                                add_face_next_cache.emplace_back(outer_prev, outer_next);
                            } else {
                                boundary_next = get_halfedge(v);
                                boundary_prev = get_prev(boundary_next);
                                add_face_next_cache.emplace_back(boundary_prev, outer_next);
                                add_face_next_cache.emplace_back(outer_prev, boundary_next);
                            }
                            break;
                        default: ;
                    }

                    // set inner link
                    add_face_next_cache.emplace_back(inner_prev, inner_next);
                } else
                    add_face_needs_adjust[ii] = (get_halfedge(v) == inner_next);

                // set face handle
                set_face(add_face_halfedges[i], f);
            }

            // process next halfedge cache
            NextCache::const_iterator ncIt(add_face_next_cache.begin()), ncEnd(add_face_next_cache.end());
            for (; ncIt != ncEnd; ++ncIt) {
                set_next(ncIt->first, ncIt->second);
            }

            // adjust vertices' halfedge handle
            for (size_t i = 0; i < N; ++i) {
                if (add_face_needs_adjust[i]) {
                    adjust_outgoing_halfedge(vertices[i]);
                }
            }

            return f;
        }

        void copy_ptrs(const PropertyContainer &vertices_,
                       const PropertyContainer &halfedges_,
                       const PropertyContainer &edges_,
                       const PropertyContainer &faces_) {
            vertices.copy_ptrs(vertices_);
            halfedges.copy_ptrs(halfedges_);
            edges.copy_ptrs(edges_);
            faces.copy_ptrs(faces_);
        }

        void init_properties() {
            vconnectivity = vertices.get_or_add<VertexConnectivity>("v:connectivity");
            hconnectivity = halfedges.get_or_add<HalfedgeConnectivity>("h:connectivity");
            fconnectivity = faces.get_or_add<FaceConnectivity>("f:connectivity");
            deleted_vertices = vertices.get_or_add<bool>("deleted_vertices");
            deleted_halfedges = halfedges.get_or_add<bool>("deleted_halfedges");
            deleted_edges = edges.get_or_add<bool>("deleted_edges");
            deleted_faces = faces.get_or_add<bool>("deleted_faces");
        }

        bool m_has_garbage = false;
        size_t m_num_deleted_vertices = 0;
        size_t m_num_deleted_edges = 0;
        size_t m_num_deleted_faces = 0;

        using NextCacheEntry = std::pair<HalfedgeHandle, HalfedgeHandle>;
        using NextCache = std::vector<NextCacheEntry>;
        std::vector<VertexHandle> m_add_face_vertices;
        std::vector<HalfedgeHandle> m_add_face_halfedges;
        std::vector<bool> m_add_face_is_new;
        std::vector<bool> m_add_face_needs_adjust;
        NextCache m_add_face_next_cache;
    };
}
