//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <glm/glm.hpp>

namespace engine {
    class Input {
    public:
        static bool mouse_captured();
        static bool keyboard_captured();

        static bool mouse_button_pressed(int key);

        static glm::vec2 mouse_position();
        static float mouse_x();
        static float mouse_y();

        static bool key_pressed(int key);
        static bool key_pressed_once(int key);
    };
}
