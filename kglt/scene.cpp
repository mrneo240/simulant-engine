#include "glee/GLee.h"
#include "scene.h"
#include "renderer.h"
#include "partitioners/null_partitioner.h"
#include "shaders/default_shaders.h"
#include "window_base.h"

namespace kglt {

Scene::Scene(WindowBase* window):
    Object(nullptr),    
    window_(window),
    default_camera_(0),
    default_scene_group_(0),
    default_texture_(0),
    default_material_(0),
    ambient_light_(1.0, 1.0, 1.0, 1.0),    
    pipeline_(new Pipeline(*this)) {

    TemplatedManager<Scene, Entity, EntityID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Entity, EntityID>));
    TemplatedManager<Scene, Camera, CameraID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Camera, CameraID>));
    TemplatedManager<Scene, ShaderProgram, ShaderID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_shader_callback));
}

Scene::~Scene() {
    //TODO: Log the unfreed resources (textures, meshes, materials etc.)
}

void Scene::initialize_defaults() {
    default_camera_ = new_camera(); //Create a default camera

    default_scene_group_ = new_scene_group();

    //Create a default pass for the default scene group
    pipeline_->add_pass(default_scene_group_);

    //Create the default blank texture
    default_texture_ = new_texture();
    Texture& tex = texture(default_texture_);
    tex.resize(1, 1);
    tex.set_bpp(32);

    tex.data()[0] = 255;
    tex.data()[1] = 255;
    tex.data()[2] = 255;
    tex.data()[3] = 255;
    tex.upload();

    default_material_ = new_material();
    Material& mat = material(default_material_);
    this->window().loader_for("kglt/materials/generic_multitexture.kglm")->into(mat);

    //Set the default material's first texture to the default (white) texture
    mat.technique().pass(0).set_texture_unit(0, tex.id());
}

EntityID Scene::new_entity(MeshID mid) {
    EntityID result = TemplatedManager<Scene, Entity, EntityID>::manager_new();

    //If a mesh was specified, set it
    if(mid) {
        entity(result).set_mesh(mid);
    }

    //Set the scene group to the default (otherwise nothing gets rendered)
    entity(result).scene_group = scene_group(default_scene_group_);

    //Tell everyone about the new entity
    signal_entity_created_(result);

    return result;
}

EntityID Scene::new_entity(Object& parent, MeshID mid) {
    EntityID result = new_entity(mid);
    entity(result).set_parent(&parent);
    return result;
}

bool Scene::has_entity(EntityID m) const {
    return TemplatedManager<Scene, Entity, EntityID>::manager_contains(m);
}

Entity& Scene::entity(EntityID e) {
    return TemplatedManager<Scene, Entity, EntityID>::manager_get(e);
}

void Scene::delete_entity(EntityID e) {
    signal_entity_destroyed_(e);

    TemplatedManager<Scene, Entity, EntityID>::manager_delete(e);
}


Mesh& Scene::mesh(MeshID m) {
    return const_cast<Mesh&>(static_cast<const Scene*>(this)->mesh(m));
}

const Mesh& Scene::mesh(MeshID m) const {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_get(m);
}

MeshID Scene::new_mesh() {
    MeshID result = TemplatedManager<Scene, Mesh, MeshID>::manager_new();
    return result;
}

bool Scene::has_mesh(MeshID m) const {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_contains(m);
}

void Scene::delete_mesh(MeshID mid) {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_delete(mid);
}

MaterialID Scene::new_material(MaterialID clone_from) {
    MaterialID result = TemplatedManager<Scene, Material, MaterialID>::manager_new();
    if(clone_from) {
        kglt::Material& existing = material(clone_from);
        kglt::Material& new_mat = material(result);

        for(uint32_t i = 0; i < existing.technique_count(); ++i) {
            //FIXME: handle multiple schemes
            MaterialTechnique& old_tech = existing.technique();
            MaterialTechnique& new_tech = new_mat.technique();
            for(uint32_t j = 0; j < old_tech.pass_count(); ++j) {
                MaterialPass& old_pass = old_tech.pass(j);
                uint32_t pass_id = new_tech.new_pass(old_pass.shader());
                MaterialPass& new_pass = new_tech.pass(pass_id);

                new_pass.set_blending(old_pass.blending());
                new_pass.set_iteration(old_pass.iteration(), old_pass.max_iterations());
                for(uint32_t k = 0; k < old_pass.texture_unit_count(); ++k) {
                    new_pass.set_texture_unit(k, old_pass.texture_unit(k).texture());
                }
                //FIXME: Copy animated texture unit and other properties
            }
        }
    }
    return result;
}

Material& Scene::material(MaterialID mid) {
    return TemplatedManager<Scene, Material, MaterialID>::manager_get(mid);
}

void Scene::delete_material(MaterialID mid) {
    TemplatedManager<Scene, Material, MaterialID>::manager_delete(mid);
}

TextureID Scene::new_texture() {
    return TemplatedManager<Scene, Texture, TextureID>::manager_new();
}

void Scene::delete_texture(TextureID tid) {
    TemplatedManager<Scene, Texture, TextureID>::manager_delete(tid);
}

Texture& Scene::texture(TextureID t) {
    return TemplatedManager<Scene, Texture, TextureID>::manager_get(t);
}

CameraID Scene::new_camera() {
    return TemplatedManager<Scene, Camera, CameraID>::manager_new();
}

Camera& Scene::camera(CameraID c) {
    if(c == 0) {
        return camera(default_camera_);
    }

    return TemplatedManager<Scene, Camera, CameraID>::manager_get(c);
}

void Scene::delete_camera(CameraID cid) {
    Camera& obj = camera(cid);
    obj.destroy_children();
    TemplatedManager<Scene, Camera, CameraID>::manager_delete(cid);
}

ShaderProgram& Scene::shader(ShaderID s) {
    return TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_get(s);
}

ShaderID Scene::new_shader() {
    return TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_new();
}

void Scene::delete_shader(ShaderID s) {
    TemplatedManager<Scene, ShaderProgram, ShaderID>::manager_delete(s);
}


SceneGroupID Scene::new_scene_group() {
    return TemplatedManager<Scene, SceneGroup, SceneGroupID>::manager_new();
}

SceneGroup& Scene::scene_group(SceneGroupID group) {
    if(group) {
        return TemplatedManager<Scene, SceneGroup, SceneGroupID>::manager_get(group);
    } else {
        return TemplatedManager<Scene, SceneGroup, SceneGroupID>::manager_get(default_scene_group_);
    }
}

void Scene::delete_scene_group(SceneGroupID group) {
    TemplatedManager<Scene, SceneGroup, SceneGroupID>::manager_delete(group);
}

LightID Scene::new_light(Object* parent, LightType type) {
    LightID lid = TemplatedManager<Scene, Light, LightID>::manager_new();

    Light& l = light(lid);
    l.set_type(type);
    if(parent) {
        l.set_parent(parent);
    }

    signal_light_created_(lid);

    return lid;
}

Light& Scene::light(LightID light_id) {
    return TemplatedManager<Scene, Light, LightID>::manager_get(light_id);
}

void Scene::delete_light(LightID light_id) {    
    Light& obj = light(light_id);
    signal_light_destroyed_(light_id);

    obj.destroy_children();
    TemplatedManager<Scene, Light, LightID>::manager_delete(light_id);
}

void Scene::init() {
    assert(glGetError() == GL_NO_ERROR);

    initialize_defaults();
    pipeline_->init(); //Initialize the pipeline

}

std::pair<ShaderID, bool> Scene::find_shader(const std::string& name) {
    std::map<std::string, ShaderID>::const_iterator it = shader_lookup_.find(name);
    if(it == shader_lookup_.end()) {
        return std::make_pair(ShaderID(), false);
    }

    return std::make_pair((*it).second, true);
}

void Scene::update(double dt) {
    /*
      Update all animated materials
    */
    for(std::pair<MaterialID, Material::ptr> p: generic::TemplatedManager<Scene, Material, MaterialID>::objects_) {
        p.second->update(dt);
    }

    for(uint32_t i = 0; i < child_count(); ++i) {
        Object& c = child(i);
        c.update(dt);
    }
}

void Scene::render() {
    pipeline_->run();
}

MeshID Scene::_mesh_id_from_mesh_ptr(Mesh* mesh) {
    return TemplatedManager<Scene, Mesh, MeshID>::_get_object_id_from_ptr(mesh);
}

}
