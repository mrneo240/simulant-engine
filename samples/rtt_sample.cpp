#include "simulant/simulant.h"

using namespace smlt;

class GameScene : public smlt::Scene<GameScene> {
public:
    GameScene(Window* window):
        smlt::Scene<GameScene>(window) {}

    void load() {
        cube_stage_ = window->new_stage();        
        auto cube_cam = cube_stage_->new_camera();

        rect_stage_ = window->new_stage();
        auto rect_cam = rect_stage_->new_camera();

        smlt::TextureID tid = window->shared_assets->new_texture_from_file("sample_data/sample.tga");
        MeshID cube_mesh = window->shared_assets->new_mesh_as_cube(1.0);
        window->shared_assets->mesh(cube_mesh)->set_texture_on_material(0, tid);
        cube_ = cube_stage_->new_actor_with_mesh(cube_mesh);
        cube_->move_to_absolute(0, 0, -4);

        MeshID rect_mesh = window->shared_assets->new_mesh_as_rectangle(2.0, 2.0);
        rect_ = rect_stage_->new_actor_with_mesh(rect_mesh);
        rect_->move_to_absolute(0, 0, -4);

        TextureID rtt = window->shared_assets->new_texture(smlt::GARBAGE_COLLECT_NEVER);
        window->shared_assets->mesh(rect_mesh)->set_texture_on_material(0, rtt);

        window->render(cube_stage_->id(), cube_cam->id()).to_texture(rtt);
        window->render(rect_stage_->id(), rect_cam->id()).to_framebuffer(
            Viewport(VIEWPORT_TYPE_FULL, smlt::Colour::GREY)
        ).with_clear();
    }

    void fixed_update(float dt) {
        cube_->rotate_y_by(Degrees(dt * 360));
        rect_->rotate_y_by(Degrees(dt * 180));
    }

private:
    StagePtr cube_stage_, rect_stage_;
    ActorPtr cube_ = nullptr;
    ActorPtr rect_ = nullptr;
};

class Sample: public smlt::Application {
public:
    Sample(const AppConfig& config):
        Application(config) {
    }

private:
    bool init() {
        scenes->register_scene<GameScene>("main");
        return true;
    }
};

int main(int argc, char* argv[]) {
    smlt::AppConfig config;
    config.title = "RTT Sample";
    config.fullscreen = false;
    config.width = 1280;
    config.height = 960;

    Sample app(config);
    return app.run();
}
