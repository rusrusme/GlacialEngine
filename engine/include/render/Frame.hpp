//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include <GLFW/glfw3.h>

namespace engine {
    class Frame {
    public:
        static void init(GLFWwindow* window, const char* glsl_version = "#version 450");

        static void shutdown();

        static void begin_frame();

        static void end_frame();
    };
}
