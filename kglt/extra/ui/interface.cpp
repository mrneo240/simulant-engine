#include <GL/GLee.h>
#include <Rocket/Core.h>
#include <Rocket/Core/SystemInterface.h>
#include <Rocket/Core/RenderInterface.h>
#include <Rocket/Core/FontDatabase.h>
#include <Rocket/Core/Vertex.h>
#include <Rocket/Core/Types.h>

#include <kazmath/mat4.h>

#include "../../kazbase/os/path.h"
#include "../../window_base.h"
#include "../../scene.h"
#include "../../camera.h"
#include "../../pipeline.h"

#include "interface.h"

namespace kglt {
namespace extra {
namespace ui {

SubScene& Interface::subscene() { return scene_.subscene(subscene_); }

class RocketSystemInterface : public Rocket::Core::SystemInterface {
public:
    RocketSystemInterface(WindowBase& window):
        window_(window) {

    }

    virtual float GetElapsedTime() {
        return window_.total_time();
    }

private:
    WindowBase& window_;
};

class RocketRenderInterface : public Rocket::Core::RenderInterface {
public:
    RocketRenderInterface(WindowBase& window):
        window_(window) {

        //At the start of each frame, clear out any rendered geometry
        window_.signal_frame_started().connect(sigc::mem_fun(this, &RocketRenderInterface::clear_objects));

    }

    bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& dimensions) {
        kglt::Texture& tex = subscene().texture(subscene().new_texture());

        uint32_t data_size = (dimensions.x * dimensions.y * 4);
        tex.resize(dimensions.x, dimensions.y);
        tex.set_bpp(32);

        tex.data().assign(source, source + data_size);
        tex.upload(true, false, false, true);

        textures_[texture_handle] = tex.id();

        return true;
    }

    void RenderGeometry(
        Rocket::Core::Vertex* vertices,
        int num_vertices,
        int* indices,
        int num_indices,
        Rocket::Core::TextureHandle texture,
        const Rocket::Core::Vector2f& translation) {

        logging::warn("Not implemented", __FILE__, __LINE__);


        kmMat4 projection;
        kmMat4OrthographicProjection(&projection, 0, subscene().window().width(), subscene().window().height(), 0, -1, 1);

        glPushMatrix();
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection.mat);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(translation.x, translation.y, 0.0);

        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        GLuint tex_id = subscene().texture(textures_[texture]).gl_tex();
        glBindTexture(GL_TEXTURE_2D, tex_id);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_ALPHA_TEST);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glBegin(GL_TRIANGLES);
        for(int32_t i = 0; i < num_indices; ++i) {
            Rocket::Core::Vertex* v = vertices + indices[i];
            glColor4ub(
                v->colour.red,
                v->colour.green,
                v->colour.blue,
                v->colour.alpha
            );
            glTexCoord2f(v->tex_coord.x, v->tex_coord.y);
            glVertex2f(v->position.x, v->position.y);
        }
        glEnd();

        glPopMatrix();
    }


    void EnableScissorRegion(bool enable) {
        logging::warn("Not implemented", __FILE__, __LINE__);
        if(enable) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    void SetScissorRegion(int x, int y, int width, int height) {
        logging::warn("Not implemented", __FILE__, __LINE__);
        glScissor(x, subscene().window().height() - (y + height), width, height);
    }

    //Our magic
    void register_context(Rocket::Core::Context* context, kglt::SubScene* subscene) {
        if(contexts_.find(context) != contexts_.end()) {
            throw std::logic_error("Tried to register the same context twice");
        }
        contexts_[context] = subscene;
    }

    void unregister_context(Rocket::Core::Context* context) {
        contexts_.erase(context);
    }

    void clear_objects() {

    }

    SubScene& subscene() {
        Rocket::Core::Context* context = GetContext();
        return *contexts_[context];
    }

private:
    WindowBase& window_;

    std::map<Rocket::Core::Context*, kglt::SubScene*> contexts_;    
    std::map<Rocket::Core::TextureHandle, kglt::TextureID> textures_;
};


static std::tr1::shared_ptr<RocketSystemInterface> rocket_system_interface_;
static std::tr1::shared_ptr<RocketRenderInterface> rocket_render_interface_;

Interface::Interface(Scene &scene, uint32_t width_in_pixels, uint32_t height_in_pixels):
    scene_(scene),
    width_(width_in_pixels),
    height_(height_in_pixels) {

    if(!rocket_system_interface_) {
        rocket_system_interface_.reset(new RocketSystemInterface(scene.window()));
        Rocket::Core::SetSystemInterface(rocket_system_interface_.get());

        rocket_render_interface_.reset(new RocketRenderInterface(scene.window()));
        Rocket::Core::SetRenderInterface(rocket_render_interface_.get());

        Rocket::Core::Initialise();

        bool font_found = false;
        for(std::string font: { "liberation/LiberationSans-Regular.ttf",
            "liberation/LiberationSans-Bold.ttf",
            "liberation/LiberationSans-Italic.ttf",
            "liberation/LiberationSans-BoldItalic.ttf"}) {
            try {
                if(!Rocket::Core::FontDatabase::LoadFontFace(locate_font(font).c_str())) {
                    throw IOError("Couldn't load the font");
                }
                font_found = true;
            } catch(IOError& e) {
                continue;
            }
        }

        if(!font_found) {
            throw IOError("Unable to find a default font");
        }
    }

    //FIXME: Change name for each interface
    context_ = Rocket::Core::CreateContext("default", Rocket::Core::Vector2i(width_in_pixels, height_in_pixels));
    subscene_ = scene.new_subscene(PARTITIONER_NULL); //Don't cull the UI

    //Register the context and subscene for with the rocket interface
    //FIXME: Need to handle subscene deletion
    rocket_render_interface_->register_context(context_, &subscene());

    camera_ = subscene().new_camera();

    subscene().camera(camera_).set_orthographic_projection(0, width_, 0, height_, -1.0, 1.0);
    scene_.pipeline().add_stage(subscene_, camera_, ViewportID(), TextureID(), kglt::RENDER_PRIORITY_FOREGROUND);

    scene.window().signal_pre_swap().connect(sigc::mem_fun(this, &Interface::render));
}

std::string Interface::locate_font(const std::string& filename) {
    //FIXME: Should be %WINDIR% not C:\Windows
    //FIXME: Should look recursively in /usr/share/fonts
    std::vector<std::string> paths = {
        "/usr/share/fonts",
        "/usr/local/share/fonts"
        "/Library/Fonts",
        "/System/Library/Fonts",
        "C:\\Windows\\fonts"
    };

    for(std::string font_dir: paths) {
        if(os::path::exists(os::path::join(font_dir, filename))) {
            return os::path::join(font_dir, filename);
        }
    }

    throw IOError("Unable to locate font: " + filename);
}

void Interface::update(float dt) {
    context_->Update();
}

void Interface::render() {
    context_->Render();
}

Interface::~Interface() {
    if(rocket_render_interface_) {
        rocket_render_interface_->unregister_context(context_);
    }
}

}
}
}
