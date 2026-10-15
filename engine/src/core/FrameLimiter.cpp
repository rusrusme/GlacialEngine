//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "core/FrameLimiter.hpp"

#include "core/Config.hpp"
#include "core/Timestep.hpp"

#if defined(_WIN32)
#include <windows.h>

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

namespace {
    HANDLE create_timer() {
        #ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
        #define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
        #endif

        HANDLE timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!timer) {
            timer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        }
        return timer;
    }

    void wait(HANDLE timer, const double seconds) {
        if (!timer || seconds <= 0.0) return;

        LARGE_INTEGER due_time;
        due_time.QuadPart = -static_cast<LONGLONG>(seconds * 10'000'000.0);

        if (SetWaitableTimer(timer, &due_time, 0, nullptr, nullptr, FALSE)) {
            WaitForSingleObject(timer, INFINITE);
        }
    }
}
#endif

namespace engine {
    FrameLimiter::FrameLimiter() {
        next_frame_time_ = Clock::current_time();

        #if defined(_WIN32)
        timeBeginPeriod(1);
        timer_ = create_timer();
        #endif
    }

    FrameLimiter::~FrameLimiter() {
        #if defined(_WIN32)
        if (timer_) {
            CloseHandle(static_cast<HANDLE>(timer_));
            timer_ = nullptr;
        }
        timeEndPeriod(1);
        #endif
    }

    void FrameLimiter::limit() {
        const int target_fps = config::video::target_fps();
        if (target_fps > 0 && target_fps < INT32_MAX) {
            const double target_frame_time = 1.0 / static_cast<double>(target_fps);
            next_frame_time_ += target_frame_time;

            const double now = Clock::current_time();
            const double time_remaining = next_frame_time_ - now;

            if (time_remaining > 0.0) {
                #if defined(_WIN32)
                wait(static_cast<HANDLE>(timer_), time_remaining);
                #else
                std::this_thread::sleep_for(std::chrono::duration<double>(time_remaining));
                #endif
            }
            else {
                next_frame_time_ = now;
            }
        }
    }
}
