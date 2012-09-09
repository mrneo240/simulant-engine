#include "glee/GLee.h"
#include "scene.h"
#include "renderer.h"
#include "ui.h"
#include "partitioners/null_partitioner.h"

namespace kglt {

const std::string get_default_vert_shader_120() {
    const std::string default_vert_shader_120 = R"(
#version 120

attribute vec3 vertex_position;
attribute vec2 vertex_texcoord_1;
attribute vec4 vertex_diffuse;
attribute vec3 vertex_normal;

uniform mat4 modelview_projection_matrix;

varying vec2 fragment_texcoord_1;
varying vec4 fragment_diffuse;

uniform vec4 light_position;

uniform float light_constant_attenuation;
uniform float light_linear_attenuation;
uniform float light_quadratic_attenuation;

varying vec3 light_direction;
varying vec3 fragment_normal;
varying vec3 eye_vec;

varying float attenuation;

void main() {
    gl_Position = modelview_projection_matrix * vec4(vertex_position, 1.0);

    light_direction = light_position.xyz - gl_Position.xyz;

    float d = length(light_direction);

    attenuation = light_constant_attenuation /
                        (1 + light_linear_attenuation*d) +
                        (1 + light_quadratic_attenuation*d*d);

    fragment_normal = vertex_normal;
    eye_vec = -gl_Position.xyz;
    fragment_texcoord_1 = vertex_texcoord_1;
    fragment_diffuse = vertex_diffuse;
}

)";

    return default_vert_shader_120;
}

const std::string get_default_frag_shader_120() {
    const std::string default_frag_shader_120 = R"(
#version 120

varying vec2 fragment_texcoord_1;
varying vec4 fragment_diffuse;

uniform sampler2D texture_1;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

varying vec3 light_direction;
varying vec3 fragment_normal;
varying vec3 eye_vec;

varying float attenuation;

void main() {
    vec4 material_ambient = vec4(1.0);
    vec4 material_diffuse = vec4(1.0);
    vec4 material_specular = vec4(1.0);
    float material_shininess = 1.0;

    vec3 N = normalize(fragment_normal);
    vec3 L = normalize(light_direction);

    float lt = dot(N, L);

    vec4 colour = (light_ambient * material_ambient);
    if(lt > 0.0) {
        colour += (light_diffuse * material_diffuse * fragment_diffuse * lt);
        vec3 E = normalize(eye_vec);
        vec3 R = reflect(-L, N);
        float specular = pow(max(dot(R, E), 0.0), material_shininess);
        colour += (light_specular * material_specular * specular);
    }

    gl_FragColor = texture2D(texture_1, fragment_texcoord_1.st) * colour * attenuation;
}

)";
    return default_frag_shader_120;
}

Scene::Scene(WindowBase* window):
    Object(nullptr),
    active_camera_(DefaultCameraID),
    window_(window),
    default_texture_(0),
    default_shader_(0),
    default_material_(0),
    background_(this),
    ui_interface_(new UI(this)),
    partitioner_(new NullPartitioner(*this)) {

    TemplatedManager<Scene, Mesh, MeshID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Mesh, MeshID>));
    TemplatedManager<Scene, Camera, CameraID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Camera, CameraID>));
    TemplatedManager<Scene, Text, TextID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Text, TextID>));
    TemplatedManager<Scene, ShaderProgram, ShaderID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_shader_callback));

    background().set_parent(this);

    active_camera_ = new_camera(); //Create a default camera

    /*
        TODO: Load the default shader which simply renders textured
        polygons like the fixed function.
    */

    //Set up the default render options
    render_options.wireframe_enabled = false;
    render_options.texture_enabled = true;
    render_options.backface_culling_enabled = true;
    render_options.point_size = 1;

    /**
     * Create the default pass, which uses a perspective projection and
     * a fullscreen viewport
     */
     add_pass(GenericRenderer::create(render_options), VIEWPORT_TYPE_FULL);
}

Scene::~Scene() {
    //TODO: Log the unfreed resources (textures, meshes, materials etc.)
}

void Scene::initialize_defaults() {
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

    //Create the default shader program
    default_shader_ = new_shader();
    ShaderProgram& def = shader(default_shader_); //Create a default shader;

    assert(glGetError() == GL_NO_ERROR);

    def.add_and_compile(SHADER_TYPE_VERTEX, get_default_vert_shader_120());
    def.add_and_compile(SHADER_TYPE_FRAGMENT, get_default_frag_shader_120());
    def.activate();

    def.params().register_auto(SP_AUTO_MODELVIEW_PROJECTION_MATRIX, "modelview_projection_matrix");
    def.params().register_auto(SP_AUTO_LIGHT_POSITION, "light_position");
    def.params().register_auto(SP_AUTO_LIGHT_AMBIENT, "light_ambient");
    def.params().register_auto(SP_AUTO_LIGHT_SPECULAR, "light_specular");
    def.params().register_auto(SP_AUTO_LIGHT_DIFFUSE, "light_diffuse");
    def.params().register_auto(SP_AUTO_LIGHT_CONSTANT_ATTENUATION, "light_constant_attenuation");
    def.params().register_auto(SP_AUTO_LIGHT_LINEAR_ATTENUATION, "light_linear_attenuation");
    def.params().register_auto(SP_AUTO_LIGHT_QUADRATIC_ATTENUATION, "light_quadratic_attenuation");

    //Bind the vertex attributes for the default shader and relink
    def.params().register_attribute(SP_ATTR_VERTEX_POSITION, "vertex_position");
    def.params().register_attribute(SP_ATTR_VERTEX_TEXCOORD0, "vertex_texcoord_1");
    def.params().register_attribute(SP_ATTR_VERTEX_COLOR, "vertex_diffuse");
    def.params().register_attribute(SP_ATTR_VERTEX_NORMAL, "vertex_normal");

    def.params().set_int("texture_1", 0); //Set texture_1 to be the first texture unit

    def.relink();

    //Finally create the default material to link them
    default_material_ = new_material();
    Material& mat = material(default_material_);
    mat.technique().new_pass(default_shader_);
    mat.technique().pass(0).set_texture_unit(0, default_texture_);
}

MeshID Scene::new_mesh(Object *parent) {
    MeshID result = TemplatedManager<Scene, Mesh, MeshID>::manager_new();
    if(parent) {
        mesh(result).set_parent(parent);
    }

    //Add the mesh to the partitioner
    partitioner_->add(mesh(result));

    return result;
}

bool Scene::has_mesh(MeshID m) const {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_contains(m);
}

Mesh& Scene::mesh(MeshID m) {
    return TemplatedManager<Scene, Mesh, MeshID>::manager_get(m);
}

void Scene::delete_mesh(MeshID mid) {
    //Remove the mesh from the partitioner
    partitioner_->remove(mesh(mid));

    Mesh& obj = mesh(mid);
    obj.destroy_children();
    return TemplatedManager<Scene, Mesh, MeshID>::manager_delete(mid);
}

MaterialID Scene::new_material() {
    return TemplatedManager<Scene, Material, MaterialID>::manager_new();
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

FontID Scene::new_font() {
    return TemplatedManager<Scene, Font, FontID>::manager_new();
}

Font& Scene::font(FontID f) {
    return TemplatedManager<Scene, Font, FontID>::manager_get(f);
}

void Scene::delete_font(FontID f) {
    TemplatedManager<Scene, Font, FontID>::manager_delete(f);
}

TextID Scene::new_text() {
    return TemplatedManager<Scene, Text, TextID>::manager_new();
}

Text& Scene::text(TextID t) {
    return TemplatedManager<Scene, Text, TextID>::manager_get(t);
}

const Text& Scene::text(TextID t) const {
    return TemplatedManager<Scene, Text, TextID>::manager_get(t);
}

void Scene::delete_text(TextID tid) {
    Text& obj = text(tid);
    obj.destroy_children();
    TemplatedManager<Scene, Text, TextID>::manager_delete(tid);
}

OverlayID Scene::new_overlay() {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_new();
}

bool Scene::has_overlay(OverlayID o) const {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_contains(o);
}

Overlay& Scene::overlay(OverlayID overlay) {
    return TemplatedManager<Scene, Overlay, OverlayID>::manager_get(overlay);
}

void Scene::delete_overlay(OverlayID oid) {
    Overlay& obj = overlay(oid);
    obj.destroy_children();
    TemplatedManager<Scene, Overlay, OverlayID>::manager_delete(oid);
}

LightID Scene::new_light(Object* parent, LightType type) {
    LightID lid = TemplatedManager<Scene, Light, LightID>::manager_new();

    Light& l = light(lid);
    l.set_type(type);
    if(parent) {
        l.set_parent(parent);
    }

    partitioner_->add(l); //Add the light to the partitioner

    return lid;
}

Light& Scene::light(LightID light_id) {
    return TemplatedManager<Scene, Light, LightID>::manager_get(light_id);
}

void Scene::delete_light(LightID light_id) {
    Light& obj = light(light_id);
    partitioner_->remove(obj); //Remove the light from the partitioner
    obj.destroy_children();
    TemplatedManager<Scene, Light, LightID>::manager_delete(light_id);
}

void Scene::init() {
    assert(glGetError() == GL_NO_ERROR);

    initialize_defaults();

}

std::pair<ShaderID, bool> Scene::find_shader(const std::string& name) {
    std::map<std::string, ShaderID>::const_iterator it = shader_lookup_.find(name);
    if(it == shader_lookup_.end()) {
        return std::make_pair(0, false);
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
    /**
     * Go through all the render passes
     * set the render options and send the viewport to OpenGL
     * before going through the scene, objects in the scene
     * should be able to mark as only being renderered in certain
     * passes
     */
    for(Pass& pass: passes_) {
        pass.renderer().set_options(render_options);
        pass.viewport().update_opengl();

        signal_render_pass_started_(pass);
        pass.renderer().render(*this);
        signal_render_pass_finished_(pass);
    }
}

MeshID Scene::_mesh_id_from_mesh_ptr(Mesh* mesh) {
    return TemplatedManager<Scene, Mesh, MeshID>::_get_object_id_from_ptr(mesh);
}

}
