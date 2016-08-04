#include "window_base.h"
#include "resource_manager.h"
#include "loader.h"
#include "procedural/mesh.h"

#include "kazbase/datetime.h"


/** FIXME
 *
 * - Write tests to show that all new_X_from_file methods mark resources as uncollected before returning
 * - Think of better GC logic, perhaps collect on every frame?
 */

namespace kglt {

ResourceManagerImpl::ResourceManagerImpl(WindowBase* window, ResourceManagerImpl *parent):
    WindowHolder(window),
    ResourceManager(window),
    parent_(parent) {

}

bool ResourceManagerImpl::init() {
    //FIXME: Should lock the default texture and material during construction!

    //Create the default blank texture
    default_texture_id_ = new_texture(GARBAGE_COLLECT_NEVER);

    auto tex = texture(default_texture_id_);

    tex->resize(1, 1);
    tex->set_bpp(32);

    tex->data()[0] = 255;
    tex->data()[1] = 255;
    tex->data()[2] = 255;
    tex->data()[3] = 255;
    tex->upload();

    //Maintain ref-count
    default_material_id_ = new_material_from_file(default_material_filename(), GARBAGE_COLLECT_NEVER);

    //Set the default material's first texture to the default (white) texture
    material(default_material_id_)->pass(0)->set_texture_unit(0, default_texture_id_);

    return true;
}

void ResourceManagerImpl::update(double dt) {
    MaterialManager::each([dt](Material* mat) {
        mat->update_controllers(dt);
        mat->update(dt);
    });

    static datetime::DateTime last_collection = datetime::now();

    if(datetime::timedelta_in_seconds(datetime::now() - last_collection) >= 5) {
        //Garbage collect all the things
        MeshManager::garbage_collect();
        MaterialManager::garbage_collect();
        TextureManager::garbage_collect();
        SoundManager::garbage_collect();

        last_collection = datetime::now();
    }
}

ProtectedPtr<Mesh> ResourceManagerImpl::mesh(MeshID m) {
    if(parent_ && !has_mesh(m)) {
        return parent_->mesh(m);
    }

    return MeshManager::manager_get(m);
}

const ProtectedPtr<Mesh> ResourceManagerImpl::mesh(MeshID m) const {
    if(parent_ && !has_mesh(m)) {
        return parent_->mesh(m);
    }

    return MeshManager::manager_get(m);
}

MeshID ResourceManagerImpl::new_mesh(VertexSpecification vertex_specification, GarbageCollectMethod garbage_collect) {
    MeshID result = MeshManager::manager_new(garbage_collect, this, vertex_specification);
    return result;
}

MeshID ResourceManagerImpl::new_mesh_from_file(const unicode& path, GarbageCollectMethod garbage_collect) {
    //Load the material
    kglt::MeshID mesh_id = new_mesh(VertexSpecification::POSITION_ONLY, garbage_collect);
    window->loader_for(path.encode())->into(mesh(mesh_id));
    MeshManager::mark_as_uncollected(mesh_id);
    return mesh_id;
}

MeshID ResourceManagerImpl::new_mesh_from_tmx_file(const unicode& tmx_file, const unicode& layer_name, float tile_render_size, GarbageCollectMethod garbage_collect) {
    kglt::MeshID mesh_id = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    window->loader_for(tmx_file.encode())->into(mesh(mesh_id), {
        {"layer", layer_name},
        {"render_size", tile_render_size}
    });
    MeshManager::mark_as_uncollected(mesh_id);
    return mesh_id;
}

MeshID ResourceManagerImpl::new_mesh_from_heightmap(const unicode& image_file, const HeightmapSpecification& spec, GarbageCollectMethod garbage_collect) {
    kglt::MeshID mesh_id = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    window->loader_for("heightmap_loader", image_file)->into(mesh(mesh_id), {
        { "spec", spec},
    });
    MeshManager::mark_as_uncollected(mesh_id);
    return mesh_id;
}

MeshID ResourceManagerImpl::new_mesh_as_cube(float width, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(
        VertexSpecification::DEFAULT,
        garbage_collect
    );
    kglt::procedural::mesh::cube(mesh(m), width);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_as_box(float width, float height, float depth, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    kglt::procedural::mesh::box(mesh(m), width, height, depth);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_as_sphere(float diameter, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    kglt::procedural::mesh::sphere(mesh(m), diameter);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_as_rectangle(float width, float height, const Vec2& offset, MaterialID material, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    kglt::procedural::mesh::rectangle(mesh(m), width, height, offset.x, offset.y, 0, false, material);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_as_cylinder(float diameter, float length, int segments, int stacks, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    kglt::procedural::mesh::cylinder(mesh(m), diameter, length, segments, stacks);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_as_capsule(float diameter, float length, int segments, int stacks, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(VertexSpecification::DEFAULT, garbage_collect);
    kglt::procedural::mesh::capsule(mesh(m), diameter, length, segments, 1, stacks);
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_from_vertices(VertexSpecification vertex_specification, const std::vector<Vec2> &vertices, MeshArrangement arrangement, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(vertex_specification, garbage_collect);

    auto new_mesh = mesh(m);
    auto smi = new_mesh->new_submesh(arrangement);
    int i = 0;
    for(auto v: vertices) {
        new_mesh->shared_data->position(v);
        new_mesh->shared_data->move_next();
        new_mesh->submesh(smi)->index_data->index(i++);
    }

    new_mesh->shared_data->done();
    new_mesh->submesh(smi)->index_data->done();

    MeshManager::mark_as_uncollected(m);

    return m;
}

MeshID ResourceManagerImpl::new_mesh_from_vertices(VertexSpecification vertex_specification, const std::vector<Vec3> &vertices, MeshArrangement arrangement, GarbageCollectMethod garbage_collect) {
    //FIXME: THis is literally a copy/paste of the function above, we can templatize this
    MeshID m = new_mesh(vertex_specification, garbage_collect);

    auto new_mesh = mesh(m);
    auto smi = new_mesh->new_submesh(arrangement);
    int i = 0;
    for(auto v: vertices) {
        new_mesh->shared_data->position(v);
        new_mesh->shared_data->move_next();
        new_mesh->submesh(smi)->index_data->index(i++);
    }

    new_mesh->shared_data->done();
    new_mesh->submesh(smi)->index_data->done();
    MeshManager::mark_as_uncollected(m);
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias(const unicode& alias, VertexSpecification vertex_specification, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh(vertex_specification, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias_from_file(const unicode& alias, const unicode& path, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh_from_file(path, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias_as_cube(const unicode& alias, float width, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh_as_cube(width, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias_as_sphere(const unicode& alias, float diameter, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh_as_sphere(diameter, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias_as_rectangle(const unicode& alias, float width, float height, const Vec2& offset, kglt::MaterialID material, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh_as_rectangle(width, height, offset, material, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }
    return m;
}

MeshID ResourceManagerImpl::new_mesh_with_alias_as_cylinder(const unicode &alias, float diameter, float length, int segments, int stacks, GarbageCollectMethod garbage_collect) {
    MeshID m = new_mesh_as_cylinder(diameter, length, segments, stacks, garbage_collect);
    try {
        MeshManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_mesh(m);
        throw;
    }

    return m;
}

MeshID ResourceManagerImpl::get_mesh_with_alias(const unicode& alias) {
    return MeshManager::manager_get_by_alias(alias);
}

void ResourceManagerImpl::delete_mesh(MeshID m) {
    mesh(m)->enable_gc();
}

bool ResourceManagerImpl::has_mesh(MeshID m) const {
    return MeshManager::manager_contains(m);
}

uint32_t ResourceManagerImpl::mesh_count() const {
    return MeshManager::manager_count();
}

MaterialID ResourceManagerImpl::new_material(GarbageCollectMethod garbage_collect) {
    MaterialID result = MaterialManager::manager_new(garbage_collect, this);
    return result;
}

void ResourceManagerImpl::delete_material(MaterialID m) {
    material(m)->enable_gc();
}

MaterialID ResourceManagerImpl::new_material_from_file(const unicode& path, GarbageCollectMethod garbage_collect) {
    /*
     * We keep a cache of the materials we've loaded from file, this massively improves performance
     * and allows sharing of the GPU program during rendering

     */
    static std::unordered_map<unicode, MaterialID> template_materials;

    /* Not in the cache? Load it from file and store as a template */
    if(!template_materials.count(path)) {
        L_INFO(_u("Loading material {0}").format(path));
        auto mat = material(new_material(GARBAGE_COLLECT_NEVER));
        window->loader_for(path.encode())->into(mat);
        template_materials[path] = mat->id();
    }

    /* Take the template, clone it, and set garbage_collection appropriately */
    auto template_id = template_materials.at(path);
    auto mat_id = material(template_id)->new_clone();
    material(mat_id)->enable_gc(garbage_collect);
    mark_material_as_uncollected(mat_id);
    return mat_id;
}

MaterialID ResourceManagerImpl::new_material_with_alias(const unicode& alias, GarbageCollectMethod garbage_collect) {
    MaterialID m = new_material(garbage_collect);

    try {
        MaterialManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_material(m);
        throw;
    }
    return m;
}

MaterialID ResourceManagerImpl::new_material_with_alias_from_file(const unicode& alias, const unicode& path, GarbageCollectMethod garbage_collect) {
    MaterialID m = new_material_from_file(path, garbage_collect);
    try {
        MaterialManager::manager_store_alias(alias, m);
    } catch(...) {
        delete_material(m);
        throw;
    }
    return m;
}

MaterialID ResourceManagerImpl::new_material_from_texture(TextureID texture_id, GarbageCollectMethod garbage_collect) {
    MaterialID m = new_material_from_file(Material::BuiltIns::TEXTURE_ONLY, garbage_collect);
    material(m)->set_texture_unit_on_all_passes(0, texture_id);
    mark_material_as_uncollected(m); //FIXME: Race-y
    return m;
}

MaterialID ResourceManagerImpl::get_material_with_alias(const unicode& alias) {
    return MaterialManager::manager_get_by_alias(alias);
}

MaterialPtr ResourceManagerImpl::material(MaterialID mid) {
    if(parent_ && !has_material(mid)) {
        return parent_->material(mid);
    }

    return MaterialManager::manager_get(mid).lock();
}

const MaterialPtr ResourceManagerImpl::material(MaterialID mid) const {
    if(parent_ && !has_material(mid)) {
        return parent_->material(mid);
    }

    return MaterialManager::manager_get(mid).lock();
}

bool ResourceManagerImpl::has_material(MaterialID m) const {
    return MaterialManager::manager_contains(m);
}

uint32_t ResourceManagerImpl::material_count() const {
    return MaterialManager::manager_count();
}

void ResourceManagerImpl::mark_material_as_uncollected(MaterialID t) {
    MaterialManager::mark_as_uncollected(t);
}

TextureID ResourceManagerImpl::new_texture(GarbageCollectMethod garbage_collect) {
    return TextureManager::manager_new(garbage_collect, this);
}

TextureID ResourceManagerImpl::new_texture_from_file(const unicode& path, TextureFlags flags, GarbageCollectMethod garbage_collect) {
    //Load the texture
    auto tex = texture(new_texture(garbage_collect));
    window->loader_for(path, LOADER_HINT_TEXTURE)->into(tex);

    if(flags.flip_vertically) {
        tex->flip_vertically();
    }

    tex->upload(
        flags.mipmap,
        flags.wrap,
        flags.filter,
        false
    );

    mark_texture_as_uncollected(tex->id());
    return tex->id();
}

void ResourceManagerImpl::delete_texture(TextureID t) {
    texture(t)->enable_gc();
}

TextureID ResourceManagerImpl::new_texture_with_alias(const unicode& alias, GarbageCollectMethod garbage_collect) {
    TextureID t = new_texture(garbage_collect);
    try {
        TextureManager::manager_store_alias(alias, t);
    } catch(...) {
        delete_texture(t);
        throw;
    }
    return t;
}

TextureID ResourceManagerImpl::new_texture_with_alias_from_file(const unicode& alias, const unicode& path, TextureFlags flags, GarbageCollectMethod garbage_collect) {
    TextureID t = new_texture_from_file(path, flags, garbage_collect);
    try {
        TextureManager::manager_store_alias(alias, t);
    } catch(...) {
        delete_texture(t);
        throw;
    }
    return t;
}

TextureID ResourceManagerImpl::get_texture_with_alias(const unicode& alias) {
    return TextureManager::manager_get_by_alias(alias);
}

TexturePtr ResourceManagerImpl::texture(TextureID t) {
    if(parent_ && !has_texture(t)) {
        return parent_->texture(t);
    }

    return TexturePtr(TextureManager::manager_get(t).lock());
}

const TexturePtr ResourceManagerImpl::texture(TextureID t) const {
    if(parent_ && !has_texture(t)) {
        return parent_->texture(t);
    }

    return TexturePtr(TextureManager::manager_get(t).lock());
}

bool ResourceManagerImpl::has_texture(TextureID t) const {
    return TextureManager::manager_contains(t);
}

uint32_t ResourceManagerImpl::texture_count() const {
    return TextureManager::manager_count();
}

void ResourceManagerImpl::mark_texture_as_uncollected(TextureID t) {
    TextureManager::mark_as_uncollected(t);
}

SoundID ResourceManagerImpl::new_sound(GarbageCollectMethod garbage_collect) {
    return SoundManager::manager_new(garbage_collect, this);
}

SoundID ResourceManagerImpl::new_sound_from_file(const unicode& path, GarbageCollectMethod garbage_collect) {
    //Load the sound
    auto snd = sound(new_sound(garbage_collect));
    window->loader_for(path.encode())->into(snd);
    return snd->id();
}

SoundID ResourceManagerImpl::new_sound_with_alias(const unicode& alias, GarbageCollectMethod garbage_collect) {
    SoundID s = new_sound(garbage_collect);
    try {
        SoundManager::manager_store_alias(alias, s);
    } catch(...) {
        delete_sound(s);
        throw;
    }
    return s;
}

SoundID ResourceManagerImpl::new_sound_with_alias_from_file(const unicode& alias, const unicode& path, GarbageCollectMethod garbage_collect) {
    SoundID s = new_sound_from_file(path, garbage_collect);
    try {
        SoundManager::manager_store_alias(alias, s);
    } catch(...) {
        delete_sound(s);
        throw;
    }
    return s;
}

SoundID ResourceManagerImpl::get_sound_with_alias(const unicode& alias) {
    return SoundManager::manager_get_by_alias(alias);
}

ProtectedPtr<Sound> ResourceManagerImpl::sound(SoundID s) {
    if(parent_ && !has_sound(s)) {
        return parent_->sound(s);
    }

    return SoundManager::manager_get(s);
}

const ProtectedPtr<Sound> ResourceManagerImpl::sound(SoundID s) const {
    if(parent_ && !has_sound(s)) {
        return parent_->sound(s);
    }

    return SoundManager::manager_get(s);
}

uint32_t ResourceManagerImpl::sound_count() const {
    return SoundManager::manager_count();
}

bool ResourceManagerImpl::has_sound(SoundID s) const {
    return SoundManager::manager_contains(s);
}

void ResourceManagerImpl::delete_sound(SoundID t) {
    sound(t)->enable_gc();
}

TextureID ResourceManagerImpl::default_texture_id() const {
    return default_texture_id_;
}

MaterialID ResourceManagerImpl::default_material_id() const {
    return default_material_id_;
}

unicode ResourceManagerImpl::default_material_filename() const {
    return window->resource_locator->locate_file(Material::BuiltIns::MULTITEXTURE2_MODULATE_WITH_LIGHTING);
}


}
