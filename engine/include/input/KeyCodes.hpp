//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    using KeyCode = int;

    namespace Key {
        // Printable ASCII range
        constexpr KeyCode SPACE            = 32;
        constexpr KeyCode APOSTROPHE       = 39;  /* ' */
        constexpr KeyCode COMMA            = 44;  /* , */
        constexpr KeyCode MINUS            = 45;  /* - */
        constexpr KeyCode PERIOD           = 46;  /* . */
        constexpr KeyCode SLASH            = 47;  /* / */
        constexpr KeyCode NUM_0            = 48;
        constexpr KeyCode NUM_1            = 49;
        constexpr KeyCode NUM_2            = 50;
        constexpr KeyCode NUM_3            = 51;
        constexpr KeyCode NUM_4            = 52;
        constexpr KeyCode NUM_5            = 53;
        constexpr KeyCode NUM_6            = 54;
        constexpr KeyCode NUM_7            = 55;
        constexpr KeyCode NUM_8            = 56;
        constexpr KeyCode NUM_9            = 57;
        constexpr KeyCode SEMICOLON        = 59;  /* ; */
        constexpr KeyCode EQUAL            = 61;  /* = */
        constexpr KeyCode A                = 65;
        constexpr KeyCode B                = 66;
        constexpr KeyCode C                = 67;
        constexpr KeyCode D                = 68;
        constexpr KeyCode E                = 69;
        constexpr KeyCode F                = 70;
        constexpr KeyCode G                = 71;
        constexpr KeyCode H                = 72;
        constexpr KeyCode I                = 73;
        constexpr KeyCode J                = 74;
        constexpr KeyCode K                = 75;
        constexpr KeyCode L                = 76;
        constexpr KeyCode M                = 77;
        constexpr KeyCode N                = 78;
        constexpr KeyCode O                = 79;
        constexpr KeyCode P                = 80;
        constexpr KeyCode Q                = 81;
        constexpr KeyCode R                = 82;
        constexpr KeyCode S                = 83;
        constexpr KeyCode T                = 84;
        constexpr KeyCode U                = 85;
        constexpr KeyCode V                = 86;
        constexpr KeyCode W                = 87;
        constexpr KeyCode X                = 88;
        constexpr KeyCode Y                = 89;
        constexpr KeyCode Z                = 90;
        constexpr KeyCode LEFT_BRACKET     = 91;  /* [ */
        constexpr KeyCode BACKSLASH        = 92;  /* \ */
        constexpr KeyCode RIGHT_BRACKET    = 93;  /* ] */
        constexpr KeyCode GRAVE_ACCENT     = 96;  /* ` */
        constexpr KeyCode WORLD_1          = 161; /* non-US #1 */
        constexpr KeyCode WORLD_2          = 162; /* non-US #2 */

        // Function & Navigation keys
        constexpr KeyCode ESCAPE           = 256;
        constexpr KeyCode ENTER            = 257;
        constexpr KeyCode TAB              = 258;
        constexpr KeyCode BACKSPACE        = 259;
        constexpr KeyCode INSERT           = 260;
        constexpr KeyCode DELETE           = 261;
        constexpr KeyCode RIGHT            = 262;
        constexpr KeyCode LEFT             = 263;
        constexpr KeyCode DOWN             = 264;
        constexpr KeyCode UP               = 265;
        constexpr KeyCode PAGE_UP          = 266;
        constexpr KeyCode PAGE_DOWN        = 267;
        constexpr KeyCode HOME             = 268;
        constexpr KeyCode END              = 269;
        constexpr KeyCode CAPS_LOCK        = 280;
        constexpr KeyCode SCROLL_LOCK      = 281;
        constexpr KeyCode NUM_LOCK         = 282;
        constexpr KeyCode PRINT_SCREEN     = 283;
        constexpr KeyCode PAUSE            = 284;

        // Function keys (F1 - F25)
        constexpr KeyCode F1               = 290;
        constexpr KeyCode F2               = 291;
        constexpr KeyCode F3               = 292;
        constexpr KeyCode F4               = 293;
        constexpr KeyCode F5               = 294;
        constexpr KeyCode F6               = 295;
        constexpr KeyCode F7               = 296;
        constexpr KeyCode F8               = 297;
        constexpr KeyCode F9               = 298;
        constexpr KeyCode F10              = 299;
        constexpr KeyCode F11              = 300;
        constexpr KeyCode F12              = 301;
        constexpr KeyCode F13              = 302;
        constexpr KeyCode F14              = 303;
        constexpr KeyCode F15              = 304;
        constexpr KeyCode F16              = 305;
        constexpr KeyCode F17              = 306;
        constexpr KeyCode F18              = 307;
        constexpr KeyCode F19              = 308;
        constexpr KeyCode F20              = 309;
        constexpr KeyCode F21              = 310;
        constexpr KeyCode F22              = 311;
        constexpr KeyCode F23              = 312;
        constexpr KeyCode F24              = 313;
        constexpr KeyCode F25              = 314;

        // Keypad
        constexpr KeyCode KP_0             = 320;
        constexpr KeyCode KP_1             = 321;
        constexpr KeyCode KP_2             = 322;
        constexpr KeyCode KP_3             = 323;
        constexpr KeyCode KP_4             = 324;
        constexpr KeyCode KP_5             = 325;
        constexpr KeyCode KP_6             = 326;
        constexpr KeyCode KP_7             = 327;
        constexpr KeyCode KP_8             = 328;
        constexpr KeyCode KP_9             = 329;
        constexpr KeyCode KP_DECIMAL       = 330;
        constexpr KeyCode KP_DIVIDE        = 331;
        constexpr KeyCode KP_MULTIPLY      = 332;
        constexpr KeyCode KP_SUBTRACT      = 333;
        constexpr KeyCode KP_ADD           = 334;
        constexpr KeyCode KP_ENTER         = 335;
        constexpr KeyCode KP_EQUAL         = 336;

        // Modifier keys
        constexpr KeyCode LEFT_SHIFT       = 340;
        constexpr KeyCode LEFT_CONTROL     = 341;
        constexpr KeyCode LEFT_ALT         = 342;
        constexpr KeyCode LEFT_SUPER       = 343;
        constexpr KeyCode RIGHT_SHIFT      = 344;
        constexpr KeyCode RIGHT_CONTROL    = 345;
        constexpr KeyCode RIGHT_ALT        = 346;
        constexpr KeyCode RIGHT_SUPER      = 347;
        constexpr KeyCode MENU             = 348;
    }
}
