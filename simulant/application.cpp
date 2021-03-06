//
//   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
//
//     This file is part of Simulant.
//
//     Simulant is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Simulant is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
//

#include <chrono>
#include <future>

#ifdef _arch_dreamcast
#include "kos_window.h"
namespace smlt { typedef KOSWindow SysWindow; }
#else
#include "sdl2_window.h"
namespace smlt { typedef SDL2Window SysWindow; }
#endif

#include "application.h"
#include "scenes/loading.h"
#include "input/input_state.h"

#define SIMULANT_PROFILE_KEY "SIMULANT_PROFILE"
#define SIMULANT_SHOW_CURSOR_KEY "SIMULANT_SHOW_CURSOR"

namespace smlt {

Application::Application(const AppConfig &config):
    config_(config) {

    construct_window(config);
}

void Application::construct_window(const AppConfig& config) {
    /* Copy to remove const */
    AppConfig config_copy = config;

    /* If we're profiling, disable the frame time and vsync */
    if(std::getenv(SIMULANT_PROFILE_KEY)) {
        config_copy.target_frame_rate = std::numeric_limits<uint16_t>::max();
        config_copy.enable_vsync = false;
    }

    /* Allow forcing the cursor at runtime */
    if(std::getenv(SIMULANT_SHOW_CURSOR_KEY)) {
        config_copy.show_cursor = true;
    }

    kazlog::get_logger("/")->add_handler(kazlog::Handler::ptr(new kazlog::StdIOHandler));
    kazlog::get_logger("/")->set_level((kazlog::LOG_LEVEL) config.log_level);

    L_DEBUG("Constructing the window");

    window_ = SysWindow::create(
        this,
        config_copy.width,
        config_copy.height,
        config_copy.bpp,
        config_copy.fullscreen,
        config_copy.enable_vsync
    );

    if(!config_copy.show_cursor) {
        // By default, don't show the cursor
        window_->show_cursor(false);

        // Lock the cursor by default
        window_->lock_cursor(true);
    } else {
        window_->show_cursor(true);
    }

    if(config_copy.target_frame_rate) {
        float frame_time = (1.0f / float(config_copy.target_frame_rate)) * 1000.0f;
        window_->request_frame_time(frame_time);
    }

    for(auto& search_path: config.search_paths) {
        window_->resource_locator->add_search_path(search_path);
    }

    L_DEBUG("Search paths added successfully");

    if(!window_->_init()) {
        throw InstanceInitializationError("Unable to create window");
    }

    window_->set_title(config.title.encode());

    /* FIXME: This is weird, the Application owns the Window, yet we're using the Window to call up to the App?
     * Not sure how to fix this without substantial changes to the frame running code */
    window_->signal_update().connect(std::bind(&Application::_call_update, this, std::placeholders::_1));
    window_->signal_late_update().connect(std::bind(&Application::_call_late_update, this, std::placeholders::_1));
    window_->signal_fixed_update().connect(std::bind(&Application::_call_fixed_update, this, std::placeholders::_1));
    window_->signal_shutdown().connect(std::bind(&Application::_call_cleanup, this));
}

StagePtr Application::stage(StageID stage) {
    return window->stage(stage);
}

bool Application::_call_init() {
    L_DEBUG("Initializing the application");

    scene_manager_.reset(new SceneManager(window_.get()));

    // Add some useful scenes by default, these can be overridden in init if the
    // user so wishes
    scenes->register_scene<scenes::Loading>("_loading");
    scenes->load("_loading");

    initialized_ = init();

    // If we successfully initialized, but the user didn't specify
    // a particular scene, we just hit the root route
    if(initialized_ && !scenes->active_scene()) {
        scenes->activate("main");
    }

    return initialized_;
}


int32_t Application::run() {
    if(!_call_init()) {
        L_ERROR("Error while initializing, terminating application");
        return 1;
    }

    while(window_->run_frame()) {}

    // Reset the scene manager (destroying scenes) before the window
    // disappears
    scene_manager_.reset();

    // Shutdown and clean up the window
    window_->_cleanup();
    window_.reset();

    return 0;
}

}
