#pragma once

#include "../generic/managed.h"
#include "../generic/tri_octree.h"
#include "../interfaces.h"
#include "../mesh.h"
#include "../sound.h"

#include "stage_node.h"

namespace smlt {

namespace impl {

    struct Triangle {
        Vec3 get_vertex(uint32_t i, void* user_data) {
            return Vec3();
        }

        group_id get_group(void* user_data) {
            return 0;
        }
    };

}


/**
 * @brief The Geom class
 *
 * A Geom is a fixed piece of geometry, like Actors they are
 * constructed from a mesh, but unlike actors they are completely
 * immovable during their lifetime. This gives partitioners
 * the freedom to split the geometry as necessary for improved performance
 * or even store entirely cached versions of the geometry.
 *
 * Also unlike an actor, a mesh is a requirement.
 */
class Geom :
    public StageNode,
    public MeshInterface,
    public Managed<Geom>,
    public generic::Identifiable<GeomID>,
    public Source {

public:
    typedef sig::signal<void (GeomID)> MeshChangedSignal;

    Geom(GeomID id, Stage* stage, MeshID mesh, const Vec3& position=Vec3(), const Quaternion rotation=Quaternion());

    MeshID mesh_id() const { return mesh_->id(); }

    const AABB aabb() const;

    void ask_owner_for_destruction();

    RenderPriority render_priority() const { return render_priority_; }

    void cleanup() override {
        StageNode::cleanup();
    }
private:
    VertexData* get_shared_data() const;

    std::shared_ptr<Mesh> mesh_;
    RenderPriority render_priority_;

    void update(double dt) {
        update_source(dt);
    }

    void compile();
};

}

