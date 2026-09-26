//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/Frame.hpp"

#include "core/Application.hpp"
#include "core/Config.hpp"
#include "render/debug/DebugFrame.hpp"

// --------------------------------------------------------------------
#if defined(_WIN32)
#include <windows.h>
// ReSharper disable once CppWrongIncludesOrder
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
namespace {
    HANDLE g_fps_timer = nullptr;

    HANDLE create_high_res_timer() {


#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

HANDLE timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!timer) {
            timer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        }
        return timer;
    }

void wait_high_res(HANDLE timer, const double seconds) {
    if (!timer || seconds <= 0.0) return;

    LARGE_INTEGER due_time;
    due_time.QuadPart = -static_cast<LONGLONG>(seconds * 10'000'000.0);

    if (SetWaitableTimer(timer, &due_time, 0, nullptr, nullptr, FALSE)) {
        WaitForSingleObject(timer, INFINITE);
    }
}
}
#endif
// --------------------------------------------------------------------

namespace engine {
    void Frame::init(void* window) {
        window_ = static_cast<GLFWwindow*>(window);

        last_frame_time_ = glfwGetTime();
        next_frame_time_ = last_frame_time_;

        // ----------------------------------
        #if defined(_WIN32)
        timeBeginPeriod(1);
        g_fps_timer = create_high_res_timer();
        #endif
        // ----------------------------------

        #ifdef ENABLE_IMGUI
        DebugFrame::init(window_);
        #endif
    }

    void Frame::shutdown() {
        #ifdef ENABLE_IMGUI
        DebugFrame::shutdown();
        #endif

        // ----------------------------------
        #if defined(_WIN32)
        if (g_fps_timer) {
            CloseHandle(g_fps_timer);
            g_fps_timer = nullptr;
        }
        timeEndPeriod(1);
        #endif
        // ----------------------------------
    }

    float Frame::begin_frame() {
        const double current_time = glfwGetTime();
        delta_time_ = static_cast<float>(current_time - last_frame_time_);
        last_frame_time_ = current_time;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        #ifdef ENABLE_IMGUI
        DebugFrame::begin_frame();
        #endif

        return delta_time_;
    }

    void Frame::end_frame() {
        #ifdef ENABLE_IMGUI
        DebugFrame::end_frame();
        #endif

        if (window_) {
            window::swap_buffers(window_);
            window::poll_events();
            // glfwSwapBuffers(window_);
            // glfwPollEvents();
        }

        limit_frame_rate();
    }

    void Frame::limit_frame_rate() {
        const int target_fps = setting::video::get_target_fps();
        if (target_fps > 0 && target_fps < INT32_MAX) {
            const double target_frame_time = 1.0 / static_cast<double>(target_fps);
            next_frame_time_ += target_frame_time;

            const double now = glfwGetTime();
            const double time_remaining = next_frame_time_ - now;

            if (time_remaining > 0.0) {
                #if defined(_WIN32)
                wait_high_res(g_fps_timer, time_remaining);
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
