#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <cstdint>

namespace kglt {

struct Vec3 {
    float x, y, z;

    Vec3():
        x(0.0f), y(0.0f), z(0.0f) {}
};

struct Vec2 {
    float x, y;
    Vec2():
        x(0.0f), y(0.0f) {}
};


//FIXME: Should be something like UniqueID<0>, UniqueID<1> or something so that
//IDs can't be incorrectly passed
typedef uint32_t MeshID;
typedef uint32_t TextureID;
typedef uint32_t CameraID;
typedef uint32_t ShaderID;
typedef uint32_t SpriteID;

const TextureID NullTextureID = 0;
const ShaderID NullShaderID = 0;
const CameraID DefaultCameraID = 0;


}

#endif // TYPES_H_INCLUDED
