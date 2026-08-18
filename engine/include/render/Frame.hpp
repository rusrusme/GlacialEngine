//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <GLFW/glfw3.h>

namespace engine {
    class Frame {
    public:
        static void init(GLFWwindow* window);
        static void shutdown();

        static float begin_frame();
        static void end_frame();

        static float get_delta_time() { return delta_time_; }

    private:
        static inline GLFWwindow* window_ = nullptr;

        static inline float delta_time_ = 0.0f;
        static inline double last_frame_time_ = 0.0;
        static inline double next_frame_time_ = 0.0;

        static void limit_frame_rate();
    };
}
