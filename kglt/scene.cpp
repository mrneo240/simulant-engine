#include "glee/GLee.h"
#include "scene.h"
#include "renderer.h"

namespace kglt {

Scene::Scene(WindowBase* window):
    active_camera_(DefaultCameraID),
    window_(window) {

    TemplatedManager<Mesh, MeshID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Mesh, MeshID>));
    TemplatedManager<Sprite, SpriteID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Sprite, SpriteID>));
    TemplatedManager<Camera, CameraID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Camera, CameraID>));
    TemplatedManager<Text, TextID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_callback<Text, TextID>));
    TemplatedManager<ShaderProgram, ShaderID>::signal_post_create().connect(sigc::mem_fun(this, &Scene::post_create_shader_callback));

    background().set_parent(this);
    ui().set_parent(this);

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
     add_pass(Renderer::ptr(new GenericRenderer(render_options)), VIEWPORT_TYPE_FULL);
}

MeshID Scene::new_mesh() {
    return TemplatedManager<Mesh, MeshID>::manager_new();
}

Mesh& Scene::mesh(MeshID m) {
    return TemplatedManager<Mesh, MeshID>::manager_get(m);
}

void Scene::delete_mesh(MeshID mid) {
    return TemplatedManager<Mesh, MeshID>::manager_delete(mid);
}

SpriteID Scene::new_sprite() {
    return TemplatedManager<Sprite, SpriteID>::manager_new();
}

Sprite& Scene::sprite(SpriteID s) {
    return TemplatedManager<Sprite, SpriteID>::manager_get(s);
}

void Scene::delete_sprite(SpriteID sid) {
    return TemplatedManager<Sprite, SpriteID>::manager_delete(sid);
}

TextureID Scene::new_texture() {
    static TextureID counter = 0;
    TextureID id = 0;
    {
        boost::mutex::scoped_lock lock(scene_lock_);
        id = ++counter;
    }
    textures_.insert(std::make_pair(id, Texture()));
    return id;
}

void Scene::delete_texture(TextureID tid) {
    {
        boost::mutex::scoped_lock lock(scene_lock_);
        textures_.erase(tid);
    }
}

Texture& Scene::texture(TextureID t) {
    if(t == NullTextureID) {
        return null_texture_;
    }
    
    boost::mutex::scoped_lock lock(scene_lock_);
    
	if(!container::contains(textures_, t)) {
		throw DoesNotExist<TextureID>();
	}
	
    return textures_[t];
}

CameraID Scene::new_camera() {
    return TemplatedManager<Camera, CameraID>::manager_new();
}

Camera& Scene::camera(CameraID c) {
    return TemplatedManager<Camera, CameraID>::manager_get(c);
}

void Scene::delete_camera(CameraID cid) {
    TemplatedManager<Camera, CameraID>::manager_delete(cid);
}

ShaderProgram& Scene::shader(ShaderID s) {
    return TemplatedManager<ShaderProgram, ShaderID>::manager_get(s);
}

ShaderID Scene::new_shader() {
    return TemplatedManager<ShaderProgram, ShaderID>::manager_new();
}

void Scene::delete_shader(ShaderID s) {
    TemplatedManager<ShaderProgram, ShaderID>::manager_delete(s);
}
    
FontID Scene::new_font() {
    static FontID counter = 0;
    FontID id = 0;
    {
        boost::mutex::scoped_lock lock(scene_lock_);
        id = ++counter;
        fonts_.insert(std::make_pair(id, Font::ptr(new Font)));
    }
    return id;
}

Font& Scene::font(FontID f) {
    boost::mutex::scoped_lock lock(scene_lock_);

    if(!container::contains(fonts_, f)) {
        throw DoesNotExist<Font>();
    }

    return *fonts_[f];
}

TextID Scene::new_text() {
    return TemplatedManager<Text, TextID>::manager_new();
}

Text& Scene::text(TextID t) {
    return TemplatedManager<Text, TextID>::manager_get(t);
}

const Text& Scene::text(TextID t) const {
    return TemplatedManager<Text, TextID>::manager_get(t);
}

void Scene::delete_text(TextID tid) {
    TemplatedManager<Text, TextID>::manager_delete(tid);
}

void Scene::init() {
    assert(glGetError() == GL_NO_ERROR);

    
    //Create the null texture
    null_texture_.resize(1, 1);
    null_texture_.set_bpp(32);
    
    null_texture_.data()[0] = 255;
    null_texture_.data()[1] = 255;
    null_texture_.data()[2] = 255;
    null_texture_.data()[3] = 255;
    null_texture_.upload();   
}

std::pair<ShaderID, bool> Scene::find_shader(const std::string& name) {
    std::map<std::string, ShaderID>::const_iterator it = shader_lookup_.find(name);
    if(it == shader_lookup_.end()) {
        return std::make_pair(NullShaderID, false);
    }

    return std::make_pair((*it).second, true);
}

void Scene::update(double dt) {
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
    return TemplatedManager<Mesh, MeshID>::_get_object_id_from_ptr(mesh);
}

}
