//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/Timestep.hpp"

namespace engine {
    float Clock::update() {
        const double current_time = glfwGetTime();
        const float delta_time = static_cast<float>(current_time - last_frame_time_);
        last_frame_time_ = current_time;
        return delta_time;
    }
} // engine