#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED

#include "../generic/managed.h"
#include "../generic/identifiable.h"
#include "../types.h"
#include "stage_node.h"

namespace smlt {

class Light :
    public StageNode,
    public generic::Identifiable<LightID>,
    public Managed<Light>,
    public std::enable_shared_from_this<Light> {

public:
    typedef std::shared_ptr<Light> ptr;

    Light(LightID lid, Stage* stage);
    void set_type(LightType type) {
        type_ = type;

        // We should never cull directional lights
        culling_mode_ = (type_ == LIGHT_TYPE_DIRECTIONAL) ?
            RENDERABLE_CULLING_MODE_NEVER : RENDERABLE_CULLING_MODE_PARTITIONER;
    }

    /*
     *  Direction (ab)uses the light's position.
     *  Setting the direction implicitly sets the light type to directional
     *
     *  Direction is stored reversed in the position.
     */
    Vec3 direction() const {
        return absolute_position();
    }

    void set_direction(float x, float y, float z) {
        set_direction(Vec3(x, y, z));
    }

    void set_direction(const kmVec3& dir) {
        set_type(LIGHT_TYPE_DIRECTIONAL);
        move_to(-dir.x, -dir.y, -dir.z);
    }

    void set_diffuse(const smlt::Colour& colour) {
        diffuse_ = colour;
    }

    void set_ambient(const smlt::Colour& colour) {
        ambient_ = colour;
    }

    void set_specular(const smlt::Colour& colour) {
        specular_ = colour;
    }

    LightType type() const { return type_; }
    smlt::Colour ambient() const { return ambient_; }
    smlt::Colour diffuse() const { return diffuse_; }
    smlt::Colour specular() const { return specular_; }

    /** Returns the owner stage's global ambient value. */
    smlt::Colour global_ambient() const { return stage->ambient_light(); }

    void set_attenuation(float range, float constant, float linear, float quadratic);
    void set_attenuation_from_range(float range);

    float range() const { return range_; }
    float constant_attenuation() const { return const_attenuation_; }
    float linear_attenuation() const { return linear_attenuation_; }
    float quadratic_attenuation() const { return quadratic_attenuation_; }

    const AABB aabb() const {
        AABB result;
        kmAABB3Initialize(&result, nullptr, range(), range(), range());
        return result;
    }

    void ask_owner_for_destruction();
    RenderableCullingMode renderable_culling_mode() const { return culling_mode_; }

    void update(double step) override {}

    void cleanup() override {
        StageNode::cleanup();
    }
private:
    LightType type_;

    smlt::Colour ambient_;
    smlt::Colour diffuse_;
    smlt::Colour specular_;

    float range_;
    float const_attenuation_;
    float linear_attenuation_;
    float quadratic_attenuation_;

    RenderableCullingMode culling_mode_ = RENDERABLE_CULLING_MODE_PARTITIONER;
};

}
#endif
