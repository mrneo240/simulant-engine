#include "../../generic/managed.h"

#include "ode_engine.h"
#include "ode_body.h"
#include "ode_collidable.h"

#include "../types.h"

namespace kglt {
namespace physics {

std::shared_ptr<ResponsiveBody> ODEEngine::new_responsive_body(Object* owner) {
    auto result = std::make_shared<ODEBody>(owner);
    if(!result->init()) {
        throw InstanceInitializationError();
    }
    return result;
}

std::shared_ptr<Collidable> ODEEngine::new_collidable(Object* owner) {
    auto result = std::make_shared<ODECollidable>(owner);
    if(!result->init()) {
        throw InstanceInitializationError();
    }
    return result;
}

ShapeID ODEEngine::create_plane(float a, float b, float c, float d) {
    ShapeID new_id = get_next_shape_id();
    dGeomID new_geom = dCreatePlane(space_, a, b, c, d);

    shapes_.insert(std::make_pair(new_id, new_geom));
    return new_id;
}

void ODEEngine::set_gravity(const Vec3& gravity) {
    dWorldSetGravity(world_, gravity.x, gravity.y, gravity.z);
}

}
}
