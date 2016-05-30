#include "batcher.h"

#include "../../actor.h"
#include "../../light.h"
#include "../../material.h"
#include "../../stage.h"
#include "../../gpu_program.h"
#include "../../camera.h"
#include "../../partitioner.h"
#include "../../window_base.h"
#include "../../utils/gl_error.h"
#include "../../utils/glcompat.h"

/*
  This is the structure of the render tree:

          ShaderNode <- GLSL shader is bound
              |
          TextureNode <- Texture units
              |
          DepthNode  <- Depth testing stuff
              |
          MaterialNode <- Uniform bindings
              |
          BlendingNode <- Blending settings
              |
          RenderSettingsNode  <- Polygon mode
              |
            Meshes
*/

namespace kglt {

void RootGroup::bind(GPUProgram *program) {

}

ProtectedPtr<CameraProxy> RootGroup::camera() {
    return stage()->camera(camera_id_);
}

StagePtr RootGroup::stage() {
    return window_.stage(stage_id_);
}

LightGroupData generate_light_group_data(GPUProgramInstance* program_instance, LightPtr light) {
    LightGroupData ret;

    auto& uniforms = program_instance->uniforms;

    ret.light_id = light->id();

    if(uniforms->uses_auto(SP_AUTO_LIGHT_POSITION)) {
        ret.light_position_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_POSITION));
        ret.light_position_value_ = Vec4(light->absolute_position(), (light->type() == LIGHT_TYPE_DIRECTIONAL) ? 0.0 : 1.0);
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_AMBIENT)) {
        ret.light_ambient_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_AMBIENT));
        ret.light_ambient_value_ = light->ambient();
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_DIFFUSE)) {
        ret.light_diffuse_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_DIFFUSE));
        ret.light_diffuse_value_ = light->diffuse();
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_SPECULAR)) {
        ret.light_specular_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_SPECULAR));
        ret.light_specular_value_ = light->specular();
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_CONSTANT_ATTENUATION)) {
        ret.light_constant_attenuation_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_CONSTANT_ATTENUATION));
        ret.light_constant_attenuation_value_ = light->constant_attenuation();
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_LINEAR_ATTENUATION)) {
        ret.light_linear_attenuation_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_LINEAR_ATTENUATION));
        ret.light_linear_attenuation_value_ = light->linear_attenuation();
    }

    if(uniforms->uses_auto(SP_AUTO_LIGHT_QUADRATIC_ATTENUATION)) {
        ret.light_quadratic_attenuation_variable_.emplace(uniforms->auto_variable_name(SP_AUTO_LIGHT_QUADRATIC_ATTENUATION));
        ret.light_quadratic_attenuation_value_ = light->quadratic_attenuation();
    }
    return ret;
}

MaterialGroupData generate_material_group_data(GPUProgramInstance* instance, MaterialPass* pass) {
    MaterialGroupData data;
    auto& uniforms = instance->uniforms;
    if(uniforms->uses_auto(SP_AUTO_MATERIAL_AMBIENT)) {
        data.ambient_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_AMBIENT);
        data.ambient = pass->ambient();
    }

    if(uniforms->uses_auto(SP_AUTO_MATERIAL_DIFFUSE)) {
        data.diffuse_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_DIFFUSE);
        data.diffuse = pass->diffuse();
    }

    if(uniforms->uses_auto(SP_AUTO_MATERIAL_SPECULAR)) {
        data.specular_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_SPECULAR);
        data.specular = pass->specular();
    }

    if(uniforms->uses_auto(SP_AUTO_MATERIAL_SHININESS)) {
        data.shininess_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_SHININESS);
        data.shininess = pass->shininess();
    }

    if(uniforms->uses_auto(SP_AUTO_MATERIAL_POINT_SIZE)) {
        data.point_size_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_POINT_SIZE);
        data.point_size = pass->point_size();
    }

    if(uniforms->uses_auto(SP_AUTO_MATERIAL_ACTIVE_TEXTURE_UNITS)) {
        data.active_texture_count_variable = uniforms->auto_variable_name(SP_AUTO_MATERIAL_ACTIVE_TEXTURE_UNITS);
        data.active_texture_count = pass->texture_unit_count();
    }

    return data;
}

void RootGroup::generate_mesh_groups(RenderGroup* parent, Renderable* renderable, MaterialPass* pass, const std::vector<LightID>& lights) {
    /*
     *  Here we add the entities to the leaves of the tree. If the Renderable can return an instanced_mesh_id we create an
     *  InstancedMeshGroup, otherwise a simple basic RenderableGroup. At the moment THERE IS NO DIFFERENCE BETWEEN THESE TWO THINGS,
     *  but it does pave the way for proper geometry instancing.
     */

    uint32_t iteration_count = 1;

    auto mesh_id = renderable->instanced_mesh_id();
    auto submesh_id = renderable->instanced_submesh_id();

    bool supports_instancing = bool(mesh_id);

    if(pass->iteration() == ITERATE_N) {
        iteration_count = pass->max_iterations();
        for(uint8_t i = 0; i < iteration_count; ++i) {
            //FIXME: What exactly is this for? Should we pass an iteration counter to the shader?

            if(supports_instancing) {
                parent->get_or_create<InstancedMeshGroup>(MeshGroupData(mesh_id, submesh_id)).add(renderable, pass);
            } else {
                parent->get_or_create<RenderableGroup>(RenderableGroupData()).add(renderable, pass);
            }
        }
    } else if (pass->iteration() == ITERATE_ONCE_PER_LIGHT) {
        iteration_count = std::min<uint32_t>(lights.size(), pass->max_iterations());
        for(uint8_t i = 0; i < iteration_count; ++i) {

            auto light = stage()->light(lights[i]);
            LightGroupData data = generate_light_group_data(pass->program.get(), light);

            auto& light_node = parent->get_or_create<LightGroup>(data);
            if(supports_instancing) {
                light_node.get_or_create<InstancedMeshGroup>(MeshGroupData(mesh_id, submesh_id)).add(renderable, pass);
            } else {
                light_node.get_or_create<RenderableGroup>(RenderableGroupData()).add(renderable, pass);
            }
        }
    } else {
        if(supports_instancing) {
            parent->get_or_create<InstancedMeshGroup>(MeshGroupData(mesh_id, submesh_id)).add(renderable, pass);
        } else {
            parent->get_or_create<RenderableGroup>(RenderableGroupData()).add(renderable, pass);
        }
    }
}

void RootGroup::insert(Renderable *renderable, MaterialPass *pass, const std::vector<LightID>& lights) {    
    auto texture_matrix_auto = [](uint8_t which) -> ShaderAvailableAuto {
        switch(which) {
        case 0: return SP_AUTO_MATERIAL_TEX_MATRIX0;
        case 1: return SP_AUTO_MATERIAL_TEX_MATRIX1;
        case 2: return SP_AUTO_MATERIAL_TEX_MATRIX2;
        case 3: return SP_AUTO_MATERIAL_TEX_MATRIX3;
        default:
            throw ValueError("Invalid tex matrix index");
        }
    };


    if(!renderable->is_visible()) return;

    auto program_instance = pass->program.get();

    //First, let's build the texture units
    RenderGroup* current = this;

    //Add a shader node
    current = &current->get_or_create<ShaderGroup>(ShaderGroupData(program_instance->_program_as_shared_ptr()));


    //Global stuff
    GlobalGroupData data;
    if(program_instance->uniforms->uses_auto(SP_AUTO_LIGHT_GLOBAL_AMBIENT)) {
        data.global_ambient_variable_.emplace(program_instance->uniforms->auto_variable_name(SP_AUTO_LIGHT_GLOBAL_AMBIENT));
    }
    current = &current->get_or_create<GlobalGroup>(data);

    //Auto attributes
    std::map<std::string, ShaderAvailableAttributes> attributes;
    /*
     * Set the attribute locations that are enabled for this pass
     */
    for(auto attribute: SHADER_AVAILABLE_ATTRS) {
        if(program_instance->attributes->uses_auto(attribute)) {
            auto varname = program_instance->attributes->variable_name(attribute);
            attributes[varname] = attribute;
        }
    }

    current = &current->get_or_create<AutoAttributeGroup>(AutoAttributeGroupData(attributes));

    //Add the texture-related branches of the tree under the shader(
    std::vector<GLuint> units(MAX_TEXTURE_UNITS, 0);
    for(uint8_t tu = 0; tu < pass->texture_unit_count(); ++tu) {
        auto& unit = pass->texture_unit(tu);
        units[tu] = stage()->texture(unit.texture_id())->gl_tex();
    }

    current = &current->get_or_create<TextureGroup>(TextureGroupData(units));

    std::vector<MatrixVariable> matrices(MAX_TEXTURE_MATRICES);
    for(uint8_t i = 0; i < MAX_TEXTURE_MATRICES; ++i) {
        if(i >= pass->texture_unit_count()) continue;
        if(program_instance->uniforms->uses_auto(texture_matrix_auto(i))) {
            auto name = program_instance->uniforms->auto_variable_name(
                ShaderAvailableAuto(SP_AUTO_MATERIAL_TEX_MATRIX0 + i)
            );

            auto& unit = pass->texture_unit(i);
            matrices[i].first.emplace(name);
            matrices[i].second = unit.matrix();
        }
    }

    current = &current->get_or_create<TextureMatrixGroup>(TextureMatrixGroupData(matrices));

    //Add a node for depth settings
    current = &current->get_or_create<DepthGroup>(DepthGroupData(pass->depth_test_enabled(), pass->depth_write_enabled()));

    //Add a node for the material properties
    current = &current->get_or_create<MaterialGroup>(generate_material_group_data(program_instance, pass));

    //Add a node for the blending type
    current = &current->get_or_create<BlendGroup>(BlendGroupData(pass->blending()));

    //Add a node for the render settings
    current = &current->get_or_create<RenderSettingsGroup>(RenderSettingsData(pass->point_size(), pass->polygon_mode()));

    //Add a node for any staged uniforms
    current = &current->get_or_create<StagedUniformGroup>(
        StagedUniformGroupData(pass->staged_float_uniforms(), pass->staged_int_uniforms())
    );

    generate_mesh_groups(current, renderable, pass, lights);
}


void GlobalGroup::bind(GPUProgram *program) {
    RootGroup& root = static_cast<RootGroup&>(get_root());

    if(data_.global_ambient_variable_) {
        program->set_uniform_colour(data_.global_ambient_variable_.value(), root.stage()->ambient_light());
    }
}

void LightGroup::bind(GPUProgram* program) {
    if(!data_.light_id) {
        return;
    }

    if(data_.light_position_variable_) {
        program->set_uniform_vec4(data_.light_position_variable_.value(), data_.light_position_value_);
    }

    if(data_.light_ambient_variable_) {
        program->set_uniform_colour(data_.light_ambient_variable_.value(), data_.light_ambient_value_);
    }

    if(data_.light_diffuse_variable_) {
        program->set_uniform_colour(data_.light_diffuse_variable_.value(), data_.light_diffuse_value_);
    }

    if(data_.light_specular_variable_) {
        program->set_uniform_colour(data_.light_specular_variable_.value(), data_.light_specular_value_);
    }

    if(data_.light_constant_attenuation_variable_) {
        program->set_uniform_float(data_.light_constant_attenuation_variable_.value(), data_.light_constant_attenuation_value_);
    }

    if(data_.light_linear_attenuation_variable_) {
        program->set_uniform_float(data_.light_linear_attenuation_variable_.value(), data_.light_linear_attenuation_value_);
    }

    if(data_.light_quadratic_attenuation_variable_) {
        program->set_uniform_float(data_.light_quadratic_attenuation_variable_.value(), data_.light_quadratic_attenuation_value_);
    }
}

void LightGroup::unbind(GPUProgram* program) {

}

void InstancedMeshGroup::bind(GPUProgram* program) {}
void InstancedMeshGroup::unbind(GPUProgram* program) {}

void ShaderGroup::bind(GPUProgram* program) {
    RootGroup& root = static_cast<RootGroup&>(get_root());

    root.set_current_program(data_.shader_);

    data_.shader_->build();
    data_.shader_->activate();
}

std::size_t ShaderGroupData::do_hash() const {
    size_t seed = 0;

    // IMPORTANT! If this hasn't been called at some point then the program object
    // always returns zero which breaks everything
    shader_->build();

    hash_combine(seed, typeid(ShaderGroupData).name());
    hash_combine(seed, shader_->program_object());
    return seed;
}

void ShaderGroup::unbind(GPUProgram *program) {
    RootGroup& root = static_cast<RootGroup&>(get_root());
    root.set_current_program(GPUProgram::ptr());
}

void AutoAttributeGroup::bind(GPUProgram *program) {
    /*
     *  This takes some explaining...
     *
     * So a GPUProgram can be shared across instances, however different instances of the same
     * program could theoretically have different attributes enabled (unlikely) or use different
     * variables. Also unfortunately changes don't take effect until you relink which is slow.
     * Fortunately you can bind more than one variable name to an attribute index (aliasing)
     * so what we do is we do is set the attribute locations for the material pass and then
     * call relink() which only links if new attribute variables or locations were found.
     * This means we may link a program a couple of times, but it'll settle down in the first
     * frame or so.
     */

    for(auto& p: data_.enabled_attributes) {
        program->set_attribute_location(p.first, (int32_t) p.second);
    }

    RootGroup& root = static_cast<RootGroup&>(get_root());
    root.current_program()->relink();
}

void AutoAttributeGroup::unbind(GPUProgram *program) {

}

void StagedUniformGroup::bind(GPUProgram *program) {
    /* It's possible to set values for arbitrary uniforms, this
     * node applies those */

    for(auto& p: data_.float_uniforms) {
        program->set_uniform_float(p.first, p.second);
    }

    for(auto& p: data_.int_uniforms) {
        program->set_uniform_int(p.first, p.second);
    }
}

void StagedUniformGroup::unbind(GPUProgram *program) {

}

void DepthGroup::bind(GPUProgram *program) {
    if(data_.depth_test) {
        GLCheck(glEnable, GL_DEPTH_TEST);
    } else {
        GLCheck(glDisable, GL_DEPTH_TEST);
    }

    if(data_.depth_write) {
        GLCheck(glDepthMask, GL_TRUE);
    } else {
        GLCheck(glDepthMask, GL_FALSE);
    }
}

void DepthGroup::unbind(GPUProgram *program) {
    if(data_.depth_test) {
        GLCheck(glDisable, GL_DEPTH_TEST);
    }
}

void TextureGroup::bind(GPUProgram* program) {
    for(uint8_t i = 0; i < MAX_TEXTURE_UNITS; ++i) {
        GLuint unit = data_.textures[i];
        GLCheck(glActiveTexture, GL_TEXTURE0 + i);
        GLCheck(glBindTexture, GL_TEXTURE_2D, unit);
    }
}

void TextureGroup::unbind(GPUProgram *program) {
    for(uint8_t i = 0; i < MAX_TEXTURE_UNITS; ++i) {
        GLCheck(glActiveTexture, GL_TEXTURE0 + i);
        GLCheck(glBindTexture, GL_TEXTURE_2D, 0);
    }
}

void TextureMatrixGroup::bind(GPUProgram *program) {
    for(uint8_t i = 0; i < MAX_TEXTURE_MATRICES; ++i) {
        auto& data = data_.texture_matrices_[i];
        if(data.first) {
            program->set_uniform_mat4x4(data.first.value(), data.second);
        }
    }
}

void TextureMatrixGroup::unbind(GPUProgram* program) {

}

void MaterialGroup::bind(GPUProgram* program) {
    if(data_.ambient_variable) {
        program->set_uniform_colour(data_.ambient_variable.value(), data_.ambient);
    }

    if(data_.diffuse_variable) {
        program->set_uniform_colour(data_.diffuse_variable.value(), data_.diffuse);
    }

    if(data_.specular_variable) {
        program->set_uniform_colour(data_.specular_variable.value(), data_.specular);
    }

    if(data_.shininess_variable) {
        program->set_uniform_float(data_.shininess_variable.value(), data_.shininess);
    }

    if(data_.point_size_variable) {
        program->set_uniform_float(data_.point_size_variable.value(), data_.point_size);
    }

    if(data_.active_texture_count_variable) {
        program->set_uniform_int(data_.active_texture_count_variable.value(), data_.active_texture_count);
    }
}

void MaterialGroup::unbind(GPUProgram *program) {

}

void BlendGroup::bind(GPUProgram* program) {
    if(data_.type == BLEND_NONE) {
        return;
    }

    GLCheck(glEnable, GL_BLEND);
    switch(data_.type) {
        case BLEND_ADD: GLCheck(glBlendFunc, GL_ONE, GL_ONE);
        break;
        case BLEND_ALPHA: GLCheck(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
        case BLEND_COLOUR: GLCheck(glBlendFunc, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
        break;
        case BLEND_MODULATE: GLCheck(glBlendFunc, GL_DST_COLOR, GL_ZERO);
        break;
        case BLEND_ONE_ONE_MINUS_ALPHA: GLCheck(glBlendFunc, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
    default:
        throw ValueError("Invalid blend type specified");
    }
}

void BlendGroup::unbind(GPUProgram *program) {
    if(data_.type != BLEND_NONE) {
        GLCheck(glDisable, GL_BLEND);
    }
}

void RenderSettingsGroup::bind(GPUProgram* program) {
#ifndef __ANDROID__
    GLCheck(glPointSize, data_.point_size);
#else
    L_WARN_ONCE("On GLES glPointSize doesn't exist");
#endif

#ifndef __ANDROID__
    switch(data_.polygon_mode) {
        case POLYGON_MODE_FILL: GLCheck(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
        break;
        case POLYGON_MODE_LINE: GLCheck(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
        break;
        case POLYGON_MODE_POINT: GLCheck(glPolygonMode, GL_FRONT_AND_BACK, GL_POINT);
        break;
    default:
        throw ValueError("Invalid polygon mode specified");
    }
#else
    if(data_.polygon_mode != POLYGON_MODE_FILL) {
        L_WARN_ONCE("On GLES glPolygonMode doesn't exist");
    }
#endif

}

void RenderSettingsGroup::unbind(GPUProgram *program) {
#ifndef __ANDROID__
    GLCheck(glPointSize, 1);
    GLCheck(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
#endif

}

}