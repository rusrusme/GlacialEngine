//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <GLFW/glfw3.h>

namespace engine {
    class Frame {
    public:
        static void on_init(GLFWwindow* window);

        static void on_shutdown();

        static void begin_frame();

        static void end_frame();
    };
}
