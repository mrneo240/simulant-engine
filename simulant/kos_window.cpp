#include <memory>
#include <vector>
#include "../deps/libgl/include/gl.h"
#include "../deps/libgl/include/glkos.h"

#include "input/input_state.h"
#include "kos_window.h"
#include "sound_drivers/kos_sound_driver.h"
#include "renderers/renderer_config.h"
#include "utils/memory.h"

namespace smlt {

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_DEPTH 32

KOSWindow::KOSWindow(uint32_t width, uint32_t height, uint32_t bpp, bool fullscreen, bool vsync_enabled):
    Window(
        width ? std::min(width, (uint32_t) SCREEN_WIDTH) : SCREEN_WIDTH,
        height ? std::min(height, (uint32_t) SCREEN_HEIGHT) : SCREEN_HEIGHT,
        bpp ? bpp : SCREEN_DEPTH, true, true) {

    platform_.reset(new DreamcastPlatform);
}


void KOSWindow::swap_buffers() {
    glKosSwapBuffers();
}

bool KOSWindow::create_window() {
    L_DEBUG("Initializing OpenGL");
#ifdef _arch_dreamcast
        print_available_ram();
#endif

    glKosInit();
    L_DEBUG("OpenGL initialized");
#ifdef _arch_dreamcast
        print_available_ram();
#endif

    renderer_ = new_renderer(this, "gl1x");

    set_has_context(true); //Mark that we have a valid GL context

    L_DEBUG("Renderer initialized");
#ifdef _arch_dreamcast
        print_available_ram();
#endif

    return true;
}

void KOSWindow::destroy_window() {

}

void KOSWindow::check_events() {
    const int8_t MAX_CONTROLLERS = 4;
    const static std::vector<uint16_t> CONTROLLER_BUTTONS = {
        CONT_A, CONT_B, CONT_C, CONT_D, CONT_X, CONT_Y, CONT_Z, CONT_START
    };

    const static std::vector<uint16_t> HAT1_BUTTONS = {
        CONT_DPAD_UP, CONT_DPAD_DOWN, CONT_DPAD_LEFT, CONT_DPAD_RIGHT
    };

    const static std::vector<uint16_t> HAT2_BUTTONS = {
        CONT_DPAD2_UP, CONT_DPAD2_DOWN, CONT_DPAD2_LEFT, CONT_DPAD2_RIGHT
    };

    static std::array<uint16_t, MAX_CONTROLLERS> previous_controller_button_state = {{0, 0, 0, 0}};
    static std::array<HatPosition, MAX_CONTROLLERS> previous_hat1_state = {
        {HAT_POSITION_CENTERED, HAT_POSITION_CENTERED, HAT_POSITION_CENTERED, HAT_POSITION_CENTERED}
    };
    static std::array<HatPosition, MAX_CONTROLLERS> previous_hat2_state = {
        {HAT_POSITION_CENTERED, HAT_POSITION_CENTERED, HAT_POSITION_CENTERED, HAT_POSITION_CENTERED}
    };

    static std::array<uint8_t, 256> previous_key_state = {}; // value-initialize to zero
    static std::array<int32_t, MAX_CONTROLLERS> previous_joyx = {{0, 0, 0, 0}};
    static std::array<int32_t, MAX_CONTROLLERS> previous_joyy = {{0, 0, 0, 0}};

    // Rescan for devices in case a controller has been added or removed
    initialize_input_controller(*this->_input_state());

    /* Check controller states */
    for(int8_t i = 0; i < MAX_CONTROLLERS; ++i) {
        auto device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if(device) {
            auto state = (cont_state_t*) maple_dev_status(device);
            if(state) {
                auto button_state = state->buttons;
                auto prev_state = previous_controller_button_state[i];
                auto joyx_state = state->joyx;
                auto joyy_state = state->joyy;

                // # Deal with the first joystick axis
                if(joyx_state != previous_joyx[i]) {
                    // joy values on the DC are -128 to 127, we scale up to the full integer range (which is then scaled to between -1.0 and 1.0)

                    float v = float(joyx_state) / 127.0f;
                    if(v < -1.0f) v  = -1.0f; // This is possible at full range to the left (due to the imbalance of -128 to 127)

                    input_state->_handle_joystick_axis_motion(i, JOYSTICK_AXIS_X, v);
                    previous_joyx[i] = joyx_state;
                }

                if(joyy_state != previous_joyy[i]) {
                    // joy values on the DC are -128 to +127, we scale up to the full integer range (which is then scaled to between -1.0 and 1.0)
                    float v = float(joyy_state) / 127.0f;
                    if(v < -1.0f) v  = -1.0f;

                    // We multiply by -1.0 as the Y-axis is reversed compared to what we expect
                    input_state->_handle_joystick_axis_motion(i, JOYSTICK_AXIS_Y, v * -1.0f);
                    previous_joyy[i] = joyy_state;
                }

                // FIXME: Add support for the second joystick (really not urgent... there's no hardware for it really...)

                // Check the current button state against the previous one
                // and update the input controller appropriately
                for(auto button: CONTROLLER_BUTTONS) {
                    if((button_state & button) && !(prev_state & button)) {
                        // Button down
                        input_state->_handle_joystick_button_down(
                            i, button
                        );
                    }

                    if(!(button_state & button) && (prev_state & button)) {
                        // Button up
                        input_state->_handle_joystick_button_up(
                            i, button
                        );
                    }
                }

                uint32_t hat1_state = (uint32_t) HAT_POSITION_CENTERED;
                for(auto button: HAT1_BUTTONS) {
                    if(button_state & button) {
                        switch(button) {
                        case CONT_DPAD_UP:
                            hat1_state |= (uint32_t) HAT_POSITION_UP;
                        break;
                        case CONT_DPAD_DOWN:
                            hat1_state |= (uint32_t) HAT_POSITION_DOWN;
                        break;
                        case CONT_DPAD_LEFT:
                            hat1_state |= (uint32_t) HAT_POSITION_LEFT;
                        break;
                        case CONT_DPAD_RIGHT:
                            hat1_state |= (uint32_t) HAT_POSITION_RIGHT;
                        break;
                        default:
                            break;
                        }
                    }
                }

                if(hat1_state != previous_hat1_state[i]) {
                    input_state->_handle_joystick_hat_motion(i, 0, (HatPosition) hat1_state);
                    previous_hat1_state[i] = (HatPosition) hat1_state;
                }

                uint32_t hat2_state = (uint32_t) HAT_POSITION_CENTERED;
                for(auto button: HAT2_BUTTONS) {
                    if(button_state & button) {
                        switch(button) {
                        case CONT_DPAD2_UP:
                            hat2_state |= (uint32_t) HAT_POSITION_UP;
                        break;
                        case CONT_DPAD2_DOWN:
                            hat2_state |= (uint32_t) HAT_POSITION_DOWN;
                        break;
                        case CONT_DPAD2_LEFT:
                            hat2_state |= (uint32_t) HAT_POSITION_LEFT;
                        break;
                        case CONT_DPAD2_RIGHT:
                            hat2_state |= (uint32_t) HAT_POSITION_RIGHT;
                        break;
                        default:
                            break;
                        }
                    }
                }

                if(hat2_state != previous_hat2_state[i]) {
                    input_state->_handle_joystick_hat_motion(i, 0, (HatPosition) hat2_state);
                    previous_hat2_state[i] = (HatPosition) hat2_state;
                }

                previous_controller_button_state[i] = button_state;
            }
        }
    }

    /* FIXME: Support multiple keyboards */

    for(int8_t i = 0; i < 1; ++i) {
        auto device = maple_enum_type(i, MAPLE_FUNC_KEYBOARD);
        if(device) {
            auto state = (kbd_state_t*) maple_dev_status(device);

            auto get_modifiers = []() -> ModifierKeyState {
                ModifierKeyState mod_state;
                //FIXME:!
                return mod_state;
            };

            if(state) {
                std::array<uint8_t, 256> key_state;
                std::copy(state->matrix, state->matrix + 256, key_state.begin());

                for(uint32_t j = 0; j < 256; ++j) {
                    if(key_state[j] && !previous_key_state[j]) {
                        // Key down
                        input_state->_handle_key_down(
                            i, KeyboardCode(j)
                        );
                        on_key_down((KeyboardCode) j, get_modifiers());
                    }
                    if(!key_state[j] && previous_key_state[j]) {
                        // Key up
                        input_state->_handle_key_up(
                            i, KeyboardCode(j)
                        );

                        on_key_up((KeyboardCode) j, get_modifiers());
                    }
                }

                previous_key_state = key_state;
            }
        }
    }

}

std::shared_ptr<SoundDriver> KOSWindow::create_sound_driver() {
    return std::make_shared<KOSSoundDriver>(this);
}

void smlt::KOSWindow::initialize_input_controller(smlt::InputState &controller) {
    std::vector<JoystickDeviceInfo> joypads;

    auto mouse_dev = maple_enum_type(0, MAPLE_FUNC_MOUSE);
    if(mouse_dev) {
        MouseDeviceInfo mouse;
        mouse.id = 0;
        mouse.button_count = 3;
        mouse.axis_count = 2;
        controller._update_mouse_devices({mouse});
    }

    auto keyboard_dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
    if(keyboard_dev) {
        KeyboardDeviceInfo keyboard;
        keyboard.id = 0;
        controller._update_keyboard_devices({keyboard});
    }

    auto controller_count = 0u;
    for(int8_t i = 0; i < 4; ++i) {
        auto device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if(device) {
            JoystickDeviceInfo info;
            info.id = i;
            info.name = device->info.product_name;
            info.button_count = 5;
            info.axis_count = 4; //2 triggers, 2 for analog
            info.hat_count = 1; // 1 D-pad
            joypads.push_back(info);

            controller_count++;
        }
    }

    controller._update_joystick_devices(joypads);
}

}
