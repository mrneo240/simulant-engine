#pragma once

#include "stage_node.h"
#include "../generic/identifiable.h"
#include "../generic/optional.h"

namespace smlt {

class RenderTarget;

class CameraProxy:
    public StageNode,
    public generic::Identifiable<CameraID>,
    public Managed<CameraProxy> {

public:
    CameraProxy(CameraID camera_id, Stage* stage);
    ~CameraProxy();

    void ask_owner_for_destruction();

    Frustum& frustum();
    smlt::optional<Vec3> project_point(const RenderTarget &target, const Viewport& viewport, const kmVec3& point);

    void set_orthographic_projection(double left, double right, double bottom, double top, double near=-1.0, double far=1.0);

    void update(double step) override;

    /* Camera Proxies have no mass/body so their AABB is just 0,0,0, or their position */
    const AABB aabb() const {
        return AABB();
    }

    const AABB transformed_aabb() const {
        return AABB(position(), position());
    }

    void cleanup() override {
        StageNode::cleanup();
    }
private:    
    CameraPtr camera();
};

}
