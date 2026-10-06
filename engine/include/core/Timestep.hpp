//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <GLFW/glfw3.h>

namespace engine {
    class Clock {
    public:
        void reset() { last_frame_time_ = glfwGetTime(); }
        float update();

    private:
        double last_frame_time_ = 0.0;
    };
} // engine
