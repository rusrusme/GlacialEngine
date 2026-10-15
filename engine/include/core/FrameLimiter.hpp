//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

namespace engine {
    class FrameLimiter {
    public:
        FrameLimiter();
        ~FrameLimiter();

        FrameLimiter(const FrameLimiter &) = delete;
        FrameLimiter & operator = (const FrameLimiter &) = delete;

        void limit();

    private:
        double next_frame_time_ = 0.0;

        #if defined(_WIN32)
        void* timer_ = nullptr;
        #endif
    };
}
