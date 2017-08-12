#pragma once

#include <SDL.h>

namespace smlt {

enum KeyboardCode {
    KEYBOARD_CODE_A = SDL_SCANCODE_A,
    KEYBOARD_CODE_B = SDL_SCANCODE_B,
    KEYBOARD_CODE_C = SDL_SCANCODE_C,
    KEYBOARD_CODE_D = SDL_SCANCODE_D,
    KEYBOARD_CODE_E = SDL_SCANCODE_E,
    KEYBOARD_CODE_F = SDL_SCANCODE_F,
    KEYBOARD_CODE_G = SDL_SCANCODE_G,
    KEYBOARD_CODE_H = SDL_SCANCODE_H,
    KEYBOARD_CODE_I = SDL_SCANCODE_I,
    KEYBOARD_CODE_J = SDL_SCANCODE_J,
    KEYBOARD_CODE_K = SDL_SCANCODE_K,
    KEYBOARD_CODE_L = SDL_SCANCODE_L,
    KEYBOARD_CODE_M = SDL_SCANCODE_M,
    KEYBOARD_CODE_N = SDL_SCANCODE_N,
    KEYBOARD_CODE_O = SDL_SCANCODE_O,
    KEYBOARD_CODE_P = SDL_SCANCODE_P,
    KEYBOARD_CODE_Q = SDL_SCANCODE_Q,
    KEYBOARD_CODE_R = SDL_SCANCODE_R,
    KEYBOARD_CODE_S = SDL_SCANCODE_S,
    KEYBOARD_CODE_T = SDL_SCANCODE_T,
    KEYBOARD_CODE_U = SDL_SCANCODE_U,
    KEYBOARD_CODE_V = SDL_SCANCODE_V,
    KEYBOARD_CODE_W = SDL_SCANCODE_W,
    KEYBOARD_CODE_X = SDL_SCANCODE_X,
    KEYBOARD_CODE_Y = SDL_SCANCODE_Y,
    KEYBOARD_CODE_Z = SDL_SCANCODE_Z,
    KEYBOARD_CODE_1 = SDL_SCANCODE_1,
    KEYBOARD_CODE_2 = SDL_SCANCODE_2,
    KEYBOARD_CODE_3 = SDL_SCANCODE_3,
    KEYBOARD_CODE_4 = SDL_SCANCODE_4,
    KEYBOARD_CODE_5 = SDL_SCANCODE_5,
    KEYBOARD_CODE_6 = SDL_SCANCODE_6,
    KEYBOARD_CODE_7 = SDL_SCANCODE_7,
    KEYBOARD_CODE_8 = SDL_SCANCODE_8,
    KEYBOARD_CODE_9 = SDL_SCANCODE_9,
    KEYBOARD_CODE_0 = SDL_SCANCODE_0,
    KEYBOARD_CODE_RETURN = SDL_SCANCODE_RETURN,
    KEYBOARD_CODE_ESCAPE = SDL_SCANCODE_ESCAPE,
    KEYBOARD_CODE_BACKSPACE = SDL_SCANCODE_BACKSPACE,
    KEYBOARD_CODE_TAB = SDL_SCANCODE_TAB,
    KEYBOARD_CODE_SPACE = SDL_SCANCODE_SPACE,
    KEYBOARD_CODE_MINUS = SDL_SCANCODE_MINUS,
    KEYBOARD_CODE_EQUALS = SDL_SCANCODE_EQUALS,
    KEYBOARD_CODE_LEFTBRACKET = SDL_SCANCODE_LEFTBRACKET,
    KEYBOARD_CODE_RIGHTBRACKET = SDL_SCANCODE_RIGHTBRACKET,
    KEYBOARD_CODE_BACKSLASH = SDL_SCANCODE_BACKSLASH,
    KEYBOARD_CODE_NONUSHASH = SDL_SCANCODE_NONUSHASH,
    KEYBOARD_CODE_SEMICOLON = SDL_SCANCODE_SEMICOLON,
    KEYBOARD_CODE_APOSTROPHE = SDL_SCANCODE_APOSTROPHE,
    KEYBOARD_CODE_GRAVE = SDL_SCANCODE_GRAVE,
    KEYBOARD_CODE_COMMA = SDL_SCANCODE_COMMA,
    KEYBOARD_CODE_PERIOD = SDL_SCANCODE_PERIOD,
    KEYBOARD_CODE_SLASH = SDL_SCANCODE_SLASH,
    KEYBOARD_CODE_CAPSLOCK = SDL_SCANCODE_CAPSLOCK,
    KEYBOARD_CODE_F1 = SDL_SCANCODE_F1,
    KEYBOARD_CODE_F2 = SDL_SCANCODE_F2,
    KEYBOARD_CODE_F3 = SDL_SCANCODE_F3,
    KEYBOARD_CODE_F4 = SDL_SCANCODE_F4,
    KEYBOARD_CODE_F5 = SDL_SCANCODE_F5,
    KEYBOARD_CODE_F6 = SDL_SCANCODE_F6,
    KEYBOARD_CODE_F7 = SDL_SCANCODE_F7,
    KEYBOARD_CODE_F8 = SDL_SCANCODE_F8,
    KEYBOARD_CODE_F9 = SDL_SCANCODE_F9,
    KEYBOARD_CODE_F10 = SDL_SCANCODE_F10,
    KEYBOARD_CODE_F11 = SDL_SCANCODE_F11,
    KEYBOARD_CODE_F12 = SDL_SCANCODE_F12,
    KEYBOARD_CODE_PRINTSCREEN = SDL_SCANCODE_PRINTSCREEN,
    KEYBOARD_CODE_SCROLLLOCK = SDL_SCANCODE_SCROLLLOCK,
    KEYBOARD_CODE_PAUSE = SDL_SCANCODE_PAUSE,
    KEYBOARD_CODE_INSERT = SDL_SCANCODE_INSERT,
    KEYBOARD_CODE_HOME = SDL_SCANCODE_HOME,
    KEYBOARD_CODE_PAGEUP = SDL_SCANCODE_PAGEUP,
    KEYBOARD_CODE_DELETE = SDL_SCANCODE_DELETE,
    KEYBOARD_CODE_END = SDL_SCANCODE_END,
    KEYBOARD_CODE_PAGEDOWN = SDL_SCANCODE_PAGEDOWN,
    KEYBOARD_CODE_RIGHT = SDL_SCANCODE_RIGHT,
    KEYBOARD_CODE_LEFT = SDL_SCANCODE_LEFT,
    KEYBOARD_CODE_DOWN = SDL_SCANCODE_DOWN,
    KEYBOARD_CODE_UP = SDL_SCANCODE_UP,
    KEYBOARD_CODE_NUMLOCKCLEAR = SDL_SCANCODE_NUMLOCKCLEAR,
    KEYBOARD_CODE_KP_DIVIDE = SDL_SCANCODE_KP_DIVIDE,
    KEYBOARD_CODE_KP_MULTIPLY = SDL_SCANCODE_KP_MULTIPLY,
    KEYBOARD_CODE_KP_MINUS = SDL_SCANCODE_KP_MINUS,
    KEYBOARD_CODE_KP_PLUS = SDL_SCANCODE_KP_PLUS,
    KEYBOARD_CODE_KP_ENTER = SDL_SCANCODE_KP_ENTER,
    KEYBOARD_CODE_KP_1 = SDL_SCANCODE_KP_1,
    KEYBOARD_CODE_KP_2 = SDL_SCANCODE_KP_2,
    KEYBOARD_CODE_KP_3 = SDL_SCANCODE_KP_3,
    KEYBOARD_CODE_KP_4 = SDL_SCANCODE_KP_4,
    KEYBOARD_CODE_KP_5 = SDL_SCANCODE_KP_5,
    KEYBOARD_CODE_KP_6 = SDL_SCANCODE_KP_6,
    KEYBOARD_CODE_KP_7 = SDL_SCANCODE_KP_7,
    KEYBOARD_CODE_KP_8 = SDL_SCANCODE_KP_8,
    KEYBOARD_CODE_KP_9 = SDL_SCANCODE_KP_9,
    KEYBOARD_CODE_KP_0 = SDL_SCANCODE_KP_0,
    KEYBOARD_CODE_KP_PERIOD = SDL_SCANCODE_KP_PERIOD,
    KEYBOARD_CODE_NONUSBACKSLASH = SDL_SCANCODE_NONUSBACKSLASH,
    KEYBOARD_CODE_APPLICATION = SDL_SCANCODE_APPLICATION,
    KEYBOARD_CODE_POWER = SDL_SCANCODE_POWER,
    KEYBOARD_CODE_KP_EQUALS = SDL_SCANCODE_KP_EQUALS,
    KEYBOARD_CODE_F13 = SDL_SCANCODE_F13,
    KEYBOARD_CODE_F14 = SDL_SCANCODE_F14,
    KEYBOARD_CODE_F15 = SDL_SCANCODE_F15,
    KEYBOARD_CODE_F16 = SDL_SCANCODE_F16,
    KEYBOARD_CODE_F17 = SDL_SCANCODE_F17,
    KEYBOARD_CODE_F18 = SDL_SCANCODE_F18,
    KEYBOARD_CODE_F19 = SDL_SCANCODE_F19,
    KEYBOARD_CODE_F20 = SDL_SCANCODE_F20,
    KEYBOARD_CODE_F21 = SDL_SCANCODE_F21,
    KEYBOARD_CODE_F22 = SDL_SCANCODE_F22,
    KEYBOARD_CODE_F23 = SDL_SCANCODE_F23,
    KEYBOARD_CODE_F24 = SDL_SCANCODE_F24,
    KEYBOARD_CODE_EXECUTE = SDL_SCANCODE_EXECUTE,
    KEYBOARD_CODE_HELP = SDL_SCANCODE_HELP,
    KEYBOARD_CODE_MENU = SDL_SCANCODE_MENU,
    KEYBOARD_CODE_SELECT = SDL_SCANCODE_SELECT,
    KEYBOARD_CODE_STOP = SDL_SCANCODE_STOP,
    KEYBOARD_CODE_AGAIN = SDL_SCANCODE_AGAIN,
    KEYBOARD_CODE_UNDO = SDL_SCANCODE_UNDO,
    KEYBOARD_CODE_CUT = SDL_SCANCODE_CUT,
    KEYBOARD_CODE_COPY = SDL_SCANCODE_COPY,
    KEYBOARD_CODE_PASTE = SDL_SCANCODE_PASTE,
    KEYBOARD_CODE_FIND = SDL_SCANCODE_FIND,
    KEYBOARD_CODE_MUTE = SDL_SCANCODE_MUTE,
    KEYBOARD_CODE_VOLUMEUP = SDL_SCANCODE_VOLUMEUP,
    KEYBOARD_CODE_VOLUMEDOWN = SDL_SCANCODE_VOLUMEDOWN,
    KEYBOARD_CODE_KP_COMMA = SDL_SCANCODE_KP_COMMA,
    KEYBOARD_CODE_KP_EQUALSAS400 = SDL_SCANCODE_KP_EQUALSAS400,
    KEYBOARD_CODE_INTERNATIONAL1 = SDL_SCANCODE_INTERNATIONAL1,
    KEYBOARD_CODE_INTERNATIONAL2 = SDL_SCANCODE_INTERNATIONAL2,
    KEYBOARD_CODE_INTERNATIONAL3 = SDL_SCANCODE_INTERNATIONAL3,
    KEYBOARD_CODE_INTERNATIONAL4 = SDL_SCANCODE_INTERNATIONAL4,
    KEYBOARD_CODE_INTERNATIONAL5 = SDL_SCANCODE_INTERNATIONAL5,
    KEYBOARD_CODE_INTERNATIONAL6 = SDL_SCANCODE_INTERNATIONAL6,
    KEYBOARD_CODE_INTERNATIONAL7 = SDL_SCANCODE_INTERNATIONAL7,
    KEYBOARD_CODE_INTERNATIONAL8 = SDL_SCANCODE_INTERNATIONAL8,
    KEYBOARD_CODE_INTERNATIONAL9 = SDL_SCANCODE_INTERNATIONAL9,
    KEYBOARD_CODE_LANG1 = SDL_SCANCODE_LANG1,
    KEYBOARD_CODE_LANG2 = SDL_SCANCODE_LANG2,
    KEYBOARD_CODE_LANG3 = SDL_SCANCODE_LANG3,
    KEYBOARD_CODE_LANG4 = SDL_SCANCODE_LANG4,
    KEYBOARD_CODE_LANG5 = SDL_SCANCODE_LANG5,
    KEYBOARD_CODE_LANG6 = SDL_SCANCODE_LANG6,
    KEYBOARD_CODE_LANG7 = SDL_SCANCODE_LANG7,
    KEYBOARD_CODE_LANG8 = SDL_SCANCODE_LANG8,
    KEYBOARD_CODE_LANG9 = SDL_SCANCODE_LANG9,
    KEYBOARD_CODE_ALTERASE = SDL_SCANCODE_ALTERASE,
    KEYBOARD_CODE_SYSREQ = SDL_SCANCODE_SYSREQ,
    KEYBOARD_CODE_CANCEL = SDL_SCANCODE_CANCEL,
    KEYBOARD_CODE_CLEAR = SDL_SCANCODE_CLEAR,
    KEYBOARD_CODE_PRIOR = SDL_SCANCODE_PRIOR,
    KEYBOARD_CODE_RETURN2 = SDL_SCANCODE_RETURN2,
    KEYBOARD_CODE_SEPARATOR = SDL_SCANCODE_SEPARATOR,
    KEYBOARD_CODE_OUT = SDL_SCANCODE_OUT,
    KEYBOARD_CODE_OPER = SDL_SCANCODE_OPER,
    KEYBOARD_CODE_CLEARAGAIN = SDL_SCANCODE_CLEARAGAIN,
    KEYBOARD_CODE_CRSEL = SDL_SCANCODE_CRSEL,
    KEYBOARD_CODE_EXSEL = SDL_SCANCODE_EXSEL,
    KEYBOARD_CODE_KP_00 = SDL_SCANCODE_KP_00,
    KEYBOARD_CODE_KP_000 = SDL_SCANCODE_KP_000,
    KEYBOARD_CODE_THOUSANDSSEPARATOR = SDL_SCANCODE_THOUSANDSSEPARATOR,
    KEYBOARD_CODE_DECIMALSEPARATOR = SDL_SCANCODE_DECIMALSEPARATOR,
    KEYBOARD_CODE_CURRENCYUNIT = SDL_SCANCODE_CURRENCYUNIT,
    KEYBOARD_CODE_CURRENCYSUBUNIT = SDL_SCANCODE_CURRENCYSUBUNIT,
    KEYBOARD_CODE_KP_LEFTPAREN = SDL_SCANCODE_KP_LEFTPAREN,
    KEYBOARD_CODE_KP_RIGHTPAREN = SDL_SCANCODE_KP_RIGHTPAREN,
    KEYBOARD_CODE_KP_LEFTBRACE = SDL_SCANCODE_KP_LEFTBRACE,
    KEYBOARD_CODE_KP_RIGHTBRACE = SDL_SCANCODE_KP_RIGHTBRACE,
    KEYBOARD_CODE_KP_TAB = SDL_SCANCODE_KP_TAB,
    KEYBOARD_CODE_KP_BACKSPACE = SDL_SCANCODE_KP_BACKSPACE,
    KEYBOARD_CODE_KP_A = SDL_SCANCODE_KP_A,
    KEYBOARD_CODE_KP_B = SDL_SCANCODE_KP_B,
    KEYBOARD_CODE_KP_C = SDL_SCANCODE_KP_C,
    KEYBOARD_CODE_KP_D = SDL_SCANCODE_KP_D,
    KEYBOARD_CODE_KP_E = SDL_SCANCODE_KP_E,
    KEYBOARD_CODE_KP_F = SDL_SCANCODE_KP_F,
    KEYBOARD_CODE_KP_XOR = SDL_SCANCODE_KP_XOR,
    KEYBOARD_CODE_KP_POWER = SDL_SCANCODE_KP_POWER,
    KEYBOARD_CODE_KP_PERCENT = SDL_SCANCODE_KP_PERCENT,
    KEYBOARD_CODE_KP_LESS = SDL_SCANCODE_KP_LESS,
    KEYBOARD_CODE_KP_GREATER = SDL_SCANCODE_KP_GREATER,
    KEYBOARD_CODE_KP_AMPERSAND = SDL_SCANCODE_KP_AMPERSAND,
    KEYBOARD_CODE_KP_DBLAMPERSAND = SDL_SCANCODE_KP_DBLAMPERSAND,
    KEYBOARD_CODE_KP_VERTICALBAR = SDL_SCANCODE_KP_VERTICALBAR,
    KEYBOARD_CODE_KP_DBLVERTICALBAR = SDL_SCANCODE_KP_DBLVERTICALBAR,
    KEYBOARD_CODE_KP_COLON = SDL_SCANCODE_KP_COLON,
    KEYBOARD_CODE_KP_HASH = SDL_SCANCODE_KP_HASH,
    KEYBOARD_CODE_KP_SPACE = SDL_SCANCODE_KP_SPACE,
    KEYBOARD_CODE_KP_AT = SDL_SCANCODE_KP_AT,
    KEYBOARD_CODE_KP_EXCLAM = SDL_SCANCODE_KP_EXCLAM,
    KEYBOARD_CODE_KP_MEMSTORE = SDL_SCANCODE_KP_MEMSTORE,
    KEYBOARD_CODE_KP_MEMRECALL = SDL_SCANCODE_KP_MEMRECALL,
    KEYBOARD_CODE_KP_MEMCLEAR = SDL_SCANCODE_KP_MEMCLEAR,
    KEYBOARD_CODE_KP_MEMADD = SDL_SCANCODE_KP_MEMADD,
    KEYBOARD_CODE_KP_MEMSUBTRACT = SDL_SCANCODE_KP_MEMSUBTRACT,
    KEYBOARD_CODE_KP_MEMMULTIPLY = SDL_SCANCODE_KP_MEMMULTIPLY,
    KEYBOARD_CODE_KP_MEMDIVIDE = SDL_SCANCODE_KP_MEMDIVIDE,
    KEYBOARD_CODE_KP_PLUSMINUS = SDL_SCANCODE_KP_PLUSMINUS,
    KEYBOARD_CODE_KP_CLEAR = SDL_SCANCODE_KP_CLEAR,
    KEYBOARD_CODE_KP_CLEARENTRY = SDL_SCANCODE_KP_CLEARENTRY,
    KEYBOARD_CODE_KP_BINARY = SDL_SCANCODE_KP_BINARY,
    KEYBOARD_CODE_KP_OCTAL = SDL_SCANCODE_KP_OCTAL,
    KEYBOARD_CODE_KP_DECIMAL = SDL_SCANCODE_KP_DECIMAL,
    KEYBOARD_CODE_KP_HEXADECIMAL = SDL_SCANCODE_KP_HEXADECIMAL,
    KEYBOARD_CODE_LCTRL = SDL_SCANCODE_LCTRL,
    KEYBOARD_CODE_LSHIFT = SDL_SCANCODE_LSHIFT,
    KEYBOARD_CODE_LALT = SDL_SCANCODE_LALT,
    KEYBOARD_CODE_LGUI = SDL_SCANCODE_LGUI,
    KEYBOARD_CODE_RCTRL = SDL_SCANCODE_RCTRL,
    KEYBOARD_CODE_RSHIFT = SDL_SCANCODE_RSHIFT,
    KEYBOARD_CODE_RALT = SDL_SCANCODE_RALT,
    KEYBOARD_CODE_RGUI = SDL_SCANCODE_RGUI,
    KEYBOARD_CODE_MODE = SDL_SCANCODE_MODE,
    KEYBOARD_CODE_AUDIONEXT = SDL_SCANCODE_AUDIONEXT,
    KEYBOARD_CODE_AUDIOPREV = SDL_SCANCODE_AUDIOPREV,
    KEYBOARD_CODE_AUDIOSTOP = SDL_SCANCODE_AUDIOSTOP,
    KEYBOARD_CODE_AUDIOPLAY = SDL_SCANCODE_AUDIOPLAY,
    KEYBOARD_CODE_AUDIOMUTE = SDL_SCANCODE_AUDIOMUTE,
    KEYBOARD_CODE_MEDIASELECT = SDL_SCANCODE_MEDIASELECT,
    KEYBOARD_CODE_WWW = SDL_SCANCODE_WWW,
    KEYBOARD_CODE_MAIL = SDL_SCANCODE_MAIL,
    KEYBOARD_CODE_CALCULATOR = SDL_SCANCODE_CALCULATOR,
    KEYBOARD_CODE_COMPUTER = SDL_SCANCODE_COMPUTER,
    KEYBOARD_CODE_AC_SEARCH = SDL_SCANCODE_AC_SEARCH,
    KEYBOARD_CODE_AC_HOME = SDL_SCANCODE_AC_HOME,
    KEYBOARD_CODE_AC_BACK = SDL_SCANCODE_AC_BACK,
    KEYBOARD_CODE_AC_FORWARD = SDL_SCANCODE_AC_FORWARD,
    KEYBOARD_CODE_AC_STOP = SDL_SCANCODE_AC_STOP,
    KEYBOARD_CODE_AC_REFRESH = SDL_SCANCODE_AC_REFRESH,
    KEYBOARD_CODE_AC_BOOKMARKS = SDL_SCANCODE_AC_BOOKMARKS,
    KEYBOARD_CODE_BRIGHTNESSDOWN = SDL_SCANCODE_BRIGHTNESSDOWN,
    KEYBOARD_CODE_BRIGHTNESSUP = SDL_SCANCODE_BRIGHTNESSUP,
    KEYBOARD_CODE_DISPLAYSWITCH = SDL_SCANCODE_DISPLAYSWITCH,
    KEYBOARD_CODE_KBDILLUMTOGGLE = SDL_SCANCODE_KBDILLUMTOGGLE,
    KEYBOARD_CODE_KBDILLUMDOWN = SDL_SCANCODE_KBDILLUMDOWN,
    KEYBOARD_CODE_KBDILLUMUP = SDL_SCANCODE_KBDILLUMUP,
    KEYBOARD_CODE_EJECT = SDL_SCANCODE_EJECT,
    KEYBOARD_CODE_SLEEP = SDL_SCANCODE_SLEEP,
    KEYBOARD_CODE_APP1 = SDL_SCANCODE_APP1,
    KEYBOARD_CODE_APP2 = SDL_SCANCODE_APP2,
    MAX_KEYBOARD_CODES = 512,
    KEYBOARD_CODE_NONE = 0
};

}
