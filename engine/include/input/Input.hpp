//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#ifdef ENABLE_IMGUI
#include <imgui.h>
#endif

#include <glm/glm.hpp>

namespace engine {
    class Input {
    public:
        static bool is_mouse_captured();
        static bool is_keyboard_captured();

        static bool is_mouse_button_pressed(int key);

        static glm::vec2 get_mouse_position();
        static float get_mouse_x();
        static float get_mouse_y();

        static bool is_key_pressed(int key);
        static bool is_key_pressed_once(int key);
    };
}
