//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Timestep.hpp"

#include <GLFW/glfw3.h>

#include "core/WindowSystem.hpp"

namespace engine {
    float Clock::update() {
        const double now = current_time();
        const auto delta_time = static_cast<float>(now - last_frame_time_);
        last_frame_time_ = now;
        return delta_time;
    }

    double Clock::current_time() {
        if (ACTIVE_FRAMEWORK == GraphicsFramework::GLFW) {
            return glfwGetTime();
        }
        if (ACTIVE_FRAMEWORK == GraphicsFramework::SDL) {
            return 0.0;
        }
        return 0.0;
    }
}