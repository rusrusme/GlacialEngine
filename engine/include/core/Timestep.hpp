//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class Clock {
    public:
        Clock() { reset(); }

        void reset() { last_frame_time_ = current_time(); }
        float update();

        static double current_time();

    private:
        double last_frame_time_ = 0.0;
    };
}
