#ifndef TEST_SOUND_H
#define TEST_SOUND_H

#include <cstdlib>
#include "simulant/simulant.h"
#include "kaztest/kaztest.h"

#include "global.h"

class SoundTest : public SimulantTestCase {
public:
    void set_up() {
#ifdef __APPLE__
	bool skip = bool(std::getenv("TRAVIS"));
	skip_if(skip, "OSX Travis builds hang on sound tests :(");
#endif

        SimulantTestCase::set_up();

        stage_ = window->new_stage();
        camera_ = stage_->new_camera();
    }

    void tear_down() {
        SimulantTestCase::tear_down();
        window->delete_stage(stage_->id());
    }

    void test_2d_sound_output() {
        smlt::SoundID sound = window->shared_assets->new_sound_from_file("test_sound.ogg");

        assert_false(window->playing_sound_count());

        window->play_sound(sound);

        assert_true(window->playing_sound_count());

        while(window->playing_sound_count()) {
            window->run_frame();
        }
    }

    void test_3d_sound_output() {
        smlt::SoundID sound = stage_->assets->new_sound_from_file("test_sound.ogg");

        auto actor = stage_->new_actor();
        actor->move_to(10, 0, 0);

        assert_false(actor->playing_sound_count());

        actor->play_sound(sound);

        assert_true(actor->playing_sound_count());

        // Finish playing the sound
        while(window->playing_sound_count()) {
            window->run_frame();
        }
    }

private:
    smlt::CameraPtr camera_;
    smlt::StagePtr stage_;

};
#endif // TEST_SOUND_H
