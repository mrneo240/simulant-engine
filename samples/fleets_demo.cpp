#include "simulant/simulant.h"

using namespace smlt;

class GameScene : public smlt::Scene<GameScene> {
public:
    GameScene(smlt::Window* window):
        smlt::Scene<GameScene>(window) {}

    void load() {
        auto pipeline = prepare_basic_scene(stage_, camera_);
        pipeline->viewport->set_colour(smlt::Colour::BLACK);

        camera_->set_perspective_projection(Degrees(45.0), float(window->width()) / float(window->height()), 10.0, 10000.0);
        ship_mesh_id_ = window->shared_assets->new_mesh_from_file("sample_data/fighter_good/space_frigate_6.obj");
        generate_ships();

        stage_->set_ambient_light(smlt::Colour(0.2, 0.2, 0.2, 1.0));
        stage_->new_light_as_directional();
    }

    void activate() {

    }

    void fixed_update(float dt) override {
        Vec3 speed(-250, 0, 0.0);
        Vec3 avg;

        for(auto ship: ships_) {
            auto pos = ship->position();
            avg += pos;
            ship->move_to_absolute(pos + (speed * dt * (0.01 * ship->id().value())));
        }

        avg /= ships_.size();

        camera_->look_at(avg);
    }

private:
    StagePtr stage_;
    CameraPtr camera_;

    MeshID ship_mesh_id_;
    std::vector<ActorPtr> ships_;

    void generate_ships() {
        Vec3 centre = Vec3(2000, 0, 0);

        auto rgen = smlt::RandomGenerator();

        for(uint32_t i = 0; i < 100; ++i) {
            Vec3 pos = centre + (Vec3(
                rgen.float_in_range(-100, 100),
                rgen.float_in_range(-100, 100),
                rgen.float_in_range(-100, 100)
            ).normalized() * rgen.float_in_range(100.0f, 150.0f));

            ships_.push_back(stage_->new_actor_with_mesh(ship_mesh_id_));
            ships_.back()->move_to_absolute(pos);

            auto ps = stage_->new_particle_system_with_parent_from_file(ships_.back()->id(), "simulant/particles/pixel_trail.kglp");
            ps->move_to(0, 0, 0);
        }
    }
};


class FleetsDemo: public smlt::Application {
public:
    FleetsDemo(const AppConfig& config):
        smlt::Application(config) {}

private:
    bool init() {
        scenes->register_scene<GameScene>("main");
        scenes->load_in_background("main", true); //Do loading in a background thread, but show immediately when done
        scenes->activate("_loading"); // Show the loading screen in the meantime
        return true;
    }
};


int main(int argc, char* argv[]) {
    AppConfig config;
    config.title = "Fleets Demo";

    FleetsDemo app(config);
    return app.run();
}
