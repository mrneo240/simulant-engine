#pragma once

#include "global.h"

namespace {

using namespace smlt;

class SkyboxTest : public SimulantTestCase {
public:
    void test_skybox_from_folder() {
        auto stage = window->new_stage();

        auto sky = stage->skies->new_skybox_from_folder("skyboxes/TropicalSunnyDay");

        assert_equal(sky->count_children(), 1u); // Should have 1 child (the actor)
    }

    void test_skybox_from_files() {

    }
};


}
