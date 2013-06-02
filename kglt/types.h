#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <memory>

#include <lua.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include "colour.h"

#include <kazmath/vec3.h>
#include <kazmath/vec2.h>
#include <kazmath/vec4.h>

#include "generic/unique_id.h"

namespace kglt {

struct Vec4 : public kmVec4 {
    Vec4() {
        kmVec4Fill(this, 0, 0, 0, 1);
    }

    Vec4(float x, float y, float z, float w) {
        kmVec4Fill(this, x, y, z, w);
    }

    Vec4(const kmVec3& v, float w) {
        kmVec4Fill(this, v.x, v.y, v.z, w);
    }
};

struct Vec2 : public kmVec2 {
    Vec2() {
        x = 0.0;
        y = 0.0;
    }

    Vec2(float x, float y) {
        kmVec2Fill(this, x, y);
    }
};

struct Vec3 : public kmVec3 {
    Vec3() {
        kmVec3Zero(this);
    }

    Vec3(float x, float y, float z) {
        kmVec3Fill(this, x, y, z);
    }

    Vec3(const kmVec2& v2, float z) {
        kmVec3Fill(this, v2.x, v2.y, z);
    }

    Vec3(const kmVec3& v) {
        kmVec3Fill(this, v.x, v.y, v.z);
    }
};

enum BlendType {
    BLEND_NONE,
    BLEND_ADD,
    BLEND_MODULATE,
    BLEND_COLOUR,
    BLEND_ALPHA,
    BLEND_ONE_ONE_MINUS_ALPHA
};

enum MeshArrangement {
    MESH_ARRANGEMENT_POINTS,
    MESH_ARRANGEMENT_TRIANGLES,
    MESH_ARRANGEMENT_TRIANGLE_FAN,
    MESH_ARRANGEMENT_TRIANGLE_STRIP,
    MESH_ARRANGEMENT_LINES,
    MESH_ARRANGEMENT_LINE_STRIP
};

enum VertexAttribute {
    VERTEX_ATTRIBUTE_POSITION = 1,
    VERTEX_ATTRIBUTE_TEXCOORD_1 = 2,
    VERTEX_ATTRIBUTE_DIFFUSE = 4,
    VERTEX_ATTRIBUTE_NORMAL = 8
};

enum AvailablePartitioner {
    PARTITIONER_NULL,
    PARTITIONER_OCTREE
};

enum LightType {
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_SPOT_LIGHT
};

enum RenderPriority {
    RENDER_PRIORITY_BACKGROUND = -100,
    RENDER_PRIORITY_DISTANT = -50,
    RENDER_PRIORITY_MAIN = 0,
    RENDER_PRIORITY_NEAR = 50,
    RENDER_PRIORITY_FOREGROUND = 100
};

const std::vector<RenderPriority> RENDER_PRIORITIES = {
    RENDER_PRIORITY_BACKGROUND,
    RENDER_PRIORITY_DISTANT,
    RENDER_PRIORITY_MAIN,
    RENDER_PRIORITY_NEAR,
    RENDER_PRIORITY_FOREGROUND
};

enum LoggingLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4
};

typedef UniqueID<0> MeshID;
typedef UniqueID<1> TextureID;
typedef UniqueID<2> CameraID;
typedef UniqueID<3> ShaderID;
typedef UniqueID<4> MaterialID;
typedef UniqueID<5> LightID;
typedef UniqueID<6> StageID;
typedef UniqueID<7> ViewportID;
typedef UniqueID<8> ActorID;
typedef UniqueID<9> SoundID;

const StageID DefaultStageID = StageID();

const std::string DEFAULT_MATERIAL_SCHEME = "default";

class Stage;
typedef std::weak_ptr<Stage> StageRef;
typedef std::shared_ptr<Stage> StagePtr;

class Mesh;
typedef std::weak_ptr<Mesh> MeshRef;
typedef std::shared_ptr<Mesh> MeshPtr;

class Material;
typedef std::weak_ptr<Material> MaterialRef;
typedef std::shared_ptr<Material> MaterialPtr;

class Texture;
typedef std::weak_ptr<Texture> TextureRef;
typedef std::shared_ptr<Texture> TexturePtr;

class Sound;
typedef std::weak_ptr<Sound> SoundRef;
typedef std::shared_ptr<Sound> SoundPtr;

class ShaderProgram;
typedef std::weak_ptr<ShaderProgram> ShaderRef;
typedef std::shared_ptr<ShaderProgram> ShaderPtr;

class Actor;
typedef std::shared_ptr<Actor> ActorPtr;
typedef std::weak_ptr<Actor> ActorRef;

class Light;
typedef std::shared_ptr<Light> LightPtr;

class Scene;
typedef std::shared_ptr<Scene> ScenePtr;

class Stage;
typedef std::shared_ptr<Stage> StagePtr;

class Camera;
typedef std::shared_ptr<Camera> CameraPtr;
typedef std::weak_ptr<Camera> CameraRef;

class Viewport;
class Frustum;
class WindowBase;
class Pipeline;
class Partitioner;


}

#endif // TYPES_H_INCLUDED
