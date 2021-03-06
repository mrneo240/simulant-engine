/* *   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
 *
 *     This file is part of Simulant.
 *
 *     Simulant is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     Simulant is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIMULANT_WINDOW_H_INCLUDED
#define SIMULANT_WINDOW_H_INCLUDED

#include <string>

#include <SDL.h>

#include "sdl2_keycodes.h"
#include "generic/managed.h"
#include "window.h"
#include "platform.h"
#include "sound_drivers/openal_sound_driver.h"

namespace smlt {

int event_filter(void* user_data, SDL_Event* event);

class SDL2Window :
    public Window {


    class SDLPlatform : public Platform {
    public:
        std::string name() const override {
#if defined(__LINUX__)
            return "linux";
#elif defined(__APPLE__)
            return "darwin";
#elif defined(__ANDROID__)
            return "android";
#elif defined(__WIN32__)
            return "windows";
#else
    #error Currently unsupported platform
#endif
        }

        void sleep_ms(uint32_t ms) const override;
    };


public:
    static Window::ptr create(Application* app, int width, int height, int bpp, bool fullscreen, bool enable_vsync) {
        return Window::create<SDL2Window>(app, width, height, bpp, fullscreen, enable_vsync);
    }

    SDL2Window(uint32_t width, uint32_t height, uint32_t bpp, bool fullscreen, bool enable_vsync);
    virtual ~SDL2Window();

    void set_title(const std::string& title);
    void show_cursor(bool value=true);
    void lock_cursor(bool cursor_locked=true);
    void cursor_position(int32_t& mouse_x, int32_t& mouse_y);
    
private:
    SDL_Window* screen_;
    SDL_GLContext context_;

    bool create_window() override;
    void destroy_window() override;

    void check_events();
    void swap_buffers();

    friend int event_filter(void* user_data, SDL_Event* event);

    void denormalize(float x, float y, int& xout, int& yout);

    std::shared_ptr<SoundDriver> create_sound_driver() override {
        return std::make_shared<OpenALSoundDriver>(this);
    }

    void initialize_input_controller(InputState &controller);

    std::vector<SDL_Joystick*> open_joysticks_;
    std::vector<SDL_GameController*> open_controllers_;
};

}

#endif // WINDOW_H_INCLUDED
