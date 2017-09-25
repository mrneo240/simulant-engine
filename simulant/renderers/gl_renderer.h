#pragma once

#include <mutex>
#include <unordered_map>
#include "../types.h"
#include "../texture.h"

#if defined(SIMULANT_GL_VERSION_1X)
    #include "gl1x/glad/glad/glad.h"
#elif defined(SIMULANT_GL_VERSION_2X)
    #include "gl2x/glad/glad/glad.h"
#endif

namespace smlt {

/*
 * Shared functionality between the GL1.x and GL2.x renderers
 * I hate mixin style classes (implementation inheritance) but as
 * we need to implement methods from the Renderer base class this
 * is the most straightforward way to share the code.
*/

class GLRenderer {
protected:
    GLRenderer(Window* window):
        win_(window) {}

    void on_texture_register(TextureID tex_id, TexturePtr texture);
    void on_texture_unregister(TextureID tex_id);
    void on_texture_prepare(TexturePtr texture);

    GLenum convert_texture_format(TextureFormat format);
    GLenum convert_texel_type(TextureTexelType type);

    std::mutex texture_object_mutex_;
    std::unordered_map<TextureID, GLuint> texture_objects_;

private:
    // Not called window_ to avoid name clashes in subclasses
    Window* win_;
};

}
