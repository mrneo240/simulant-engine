#ifndef TEST_MATERIAL_H
#define TEST_MATERIAL_H

#include "kglt/kazbase/testing.h"

#include "kglt/kglt.h"
#include "global.h"

class MaterialTest : public TestCase {
public:
    void set_up() {
        if(!window) {
            window = kglt::Window::create();
            window->set_logging_level(LOG_LEVEL_NONE);
        }

        //window->reset();
    }

    void test_material_initialization() {
        kglt::Scene& scene = window->scene();

        kglt::Material& mat = scene.material(scene.new_material());

        this->assert_equal((uint32_t)1, mat.technique_count()); //Should return the default technique
        this->assert_equal(kglt::DEFAULT_MATERIAL_SCHEME, mat.technique().scheme());
        mat.technique().new_pass(ShaderID()); //Create a pass
        this->assert_equal((uint32_t)1, mat.technique().pass_count()); //Should return the default pass
        this->assert_true(kglt::Colour::white == mat.technique().pass(0).diffuse()); //this->assert_true the default pass sets white as the default
        this->assert_true(kglt::Colour::white == mat.technique().pass(0).ambient()); //this->assert_true the default pass sets white as the default
        this->assert_true(kglt::Colour::white == mat.technique().pass(0).specular()); //this->assert_true the default pass sets white as the default
        this->assert_equal(0.0, mat.technique().pass(0).shininess());
    }

    void test_material_applies_to_mesh() {
        kglt::Scene& scene = window->scene();

        kglt::MaterialID mid = scene.new_material();
        kglt::MeshID mesh_id = scene.new_mesh();
        kglt::Mesh& mesh = scene.mesh(mesh_id);
        kglt::SubMeshIndex idx = mesh.new_submesh(mid);
        this->assert_equal(mid, mesh.submesh(idx).material());
    }

    void test_reflectiveness() {
        kglt::Scene& scene = window->scene();

        kglt::MaterialID mid = scene.new_material();
        uint32_t pass_id = scene.material(mid).technique().new_pass(ShaderID());
        kglt::MaterialPass& pass = scene.material(mid).technique().pass(pass_id);

        assert_false(pass.is_reflective());
        assert_false(scene.material(mid).technique().has_reflective_pass());
        assert_equal(0.0, pass.albedo());
        assert_equal(0, pass.reflection_texture_unit());

        pass.set_albedo(0.5);

        assert_equal(0.5, pass.albedo());
        assert_true(pass.is_reflective());
        assert_true(scene.material(mid).technique().has_reflective_pass());
    }
};

#endif // TEST_MATERIAL_H