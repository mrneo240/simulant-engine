
#include "shader.h"

namespace GL {

ShaderProgram::ShaderProgram() {

    for(uint32_t i = 0; i < MAX_SHADER_TYPES; ++i) {
        shader_ids_[i] = 0;
    }

    glCreateProgram(program_id_);

}

ShaderProgram::~ShaderProgram() {

}

void ShaderProgram::activate() {
    glUseProgram(program_id_);
}

void ShaderProgram::add_and_compile(ShaderType type, const std::string& source) {
    if(shader_ids_[type]) {
        glDeleteShader(shader_ids_[type]);
        shader_ids_[type] = 0;
    }

    GLenum shader_type;
    switch(type) {
        case ShaderType::VERTEX {
            shader_type = GL_VERTEX_SHADER;
        } break;
        case ShaderType::FRAGMENT {
            shader_type = GL_FRAGMENT_SHADER;
        } break;
        default:
            throw std::logic_error("Invalid shader type");
    }

    shader_ids_[type] = glCreateShader(shader_type)

    const char* c_str = source.c_str();
    glShaderSource(shader_ids_[type], 1, &c_str, nullptr);
    glCompileShader(shader_ids_[type]);

    glAttachShader(program_id_, shader_ids_[type]);
    glLinkProgram(program_id_);
}

void ShaderProgram::set_uniform(const std::string& name, const float x) {

}

void ShaderProgram::set_uniform(const std::string& name, const int32_t x) {

}

void ShaderProgram::set_uniform(const std::string& name, const kmMat4* matrix) {

}

void ShaderProgram::set_uniform(const std::string& name, const kmMat3* matrix) {

}

void ShaderProgram::set_uniform(const std::string& name, const kmVec3* vec) {

}

}
