//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "core/Application.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <core/Config.hpp>
#include <core/Log.hpp>
#include <GLFW/glfw3.h>
#include <input/CameraController.hpp>
#include <input/Input.hpp>
#include <render/Camera.hpp>
#include <render/Frame.hpp>

// --------------------------------------------------------------------
#if defined(_WIN32)
#include <windows.h>
// ReSharper disable once CppWrongIncludesOrder
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
namespace {
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
    namespace {
        struct ApplicationContext {
            Renderer* renderer = nullptr;
            CameraController* camera_controller = nullptr;
        };

        void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
            const auto* context = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(window));
            if (context != nullptr && context->renderer != nullptr) {
                context->renderer->on_resize(width, height);

                if (height > 0) {
                    const float aspect = static_cast<float>(width) / static_cast<float>(height);
                    context->renderer->get_camera().set_aspect_ratio(aspect);
                }
            }
        }

        void scroll_callback(GLFWwindow* window, double /*x_offset*/, const double y_offset) {
            if (Input::is_mouse_captured()) {
                return;
            }
            const auto* context = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(window));
            if (context != nullptr && context->renderer != nullptr && context->camera_controller != nullptr) {
                CameraController::on_scroll(context->renderer->get_camera(), y_offset);
            }
        }
    }

    int Application::run() {
        if (!glfwInit()) {
            std::cerr << "Engine error: application: failed to initialize GLFW" << std::endl;
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(
            setting::video::MONITOR_RESOLUTION_WIDTH,
            setting::video::MONITOR_RESOLUTION_HEIGHT,
            setting::FRAME_TITLE, nullptr, nullptr);
        if (!window) {
            std::cerr << "Engine error: application: failed to create window" << std::endl;
            glfwTerminate();
            return 1;
        }

        glfwMakeContextCurrent(window);

        // ----------------------------------
        // Initialize logger
        // ----------------------------------
        Log::init();

        // ----------------------------------
        // Initialize frame
        // ----------------------------------
        Frame::on_init(window);

        Renderer renderer;
        CameraController camera_controller;
        ApplicationContext context;
        context.renderer = &renderer;
        context.camera_controller = &camera_controller;

        if (!renderer.on_init(window)) {
            std::cerr << "Engine error: application: failed to initialize renderer" << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }

        glfwSetWindowUserPointer(window, &context);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetScrollCallback(window, scroll_callback);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        for (const auto& layer : layers_) {
            layer->on_attach(renderer);
        }
        if (height > 0) {
            renderer.get_camera().set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        }
        renderer.get_camera().set_projection_params(45.0f, 0.1f, 10000.0f);

        // ----------------------------------
        // Game logic
        // ----------------------------------
        setting::configuration::load();
        this->on_init(renderer);

        // ----------------------------------
        #if defined(_WIN32)
        timeBeginPeriod(1);
        HANDLE fps_timer = create_high_res_timer();
        #endif
        // ----------------------------------

        double last_frame_time = glfwGetTime();
        double next_frame_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            // ----------------------------------
            // Delta timer
            // ----------------------------------
            const double current_time = glfwGetTime();
            if (last_frame_time == 0.0) {
                last_frame_time = current_time;
            }
            const auto delta_time = static_cast<float>(current_time - last_frame_time);
            last_frame_time = current_time;

            // ----------------------------------
            // Update pass
            // ----------------------------------
            Frame::begin_frame();
            if (!Input::is_mouse_captured()) {
                camera_controller.update(window, renderer.get_camera(), delta_time);
            }
            else {
                camera_controller.reset_mouse_state();
            }
            for (const auto& layer : layers_) {
                layer->on_update(renderer, window, camera_controller, delta_time);
            }
            this->on_update(renderer, window, camera_controller, delta_time);

            // ----------------------------------
            // Render pass
            // ----------------------------------
            renderer.begin_frame();
            for (const auto& layer : layers_) {
                layer->on_render(renderer);
            }
            this->on_render(renderer);

            #ifdef ENABLE_IMGUI
            for (const auto& layer : layers_) {
                layer->on_imGuiRender(delta_time);
            }
            #endif

            renderer.end_frame(delta_time);
            Frame::end_frame();

            glfwSwapBuffers(window);
            glfwPollEvents();

            // ----------------------------------
            // Frame rate limiter
            // ----------------------------------
            const int target_fps = setting::video::get_target_fps();
            if (target_fps > 0 && target_fps < INT32_MAX) {
                const double target_frame_time = 1.0 / static_cast<double>(target_fps);
                next_frame_time += target_frame_time;

                const double now = glfwGetTime();
                const double time_remaining = next_frame_time - now;

                if (time_remaining > 0.0) {
                    #if defined(_WIN32)
                    wait_high_res(fps_timer, time_remaining);
                    #else
                    std::this_thread::sleep_for(std::chrono::duration<double>(time_remaining));
                    #endif
                }
                else {
                    next_frame_time = now;
                }
            }
        }
        // ----------------------------------
        #if defined(_WIN32)
        if (fps_timer) {
            CloseHandle(fps_timer);
        }
        timeEndPeriod(1);
        #endif
        // ----------------------------------

        // In reverse
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_detach();
        }

        this->on_shutdown(renderer, window);
        return 0;
    }

    void Application::on_shutdown(Renderer& renderer, GLFWwindow* window) {
        setting::configuration::save();

        Frame::on_shutdown();
        renderer.on_shutdown();
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layers_.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        layers_.push_overlay(std::move(overlay));
    }
}
