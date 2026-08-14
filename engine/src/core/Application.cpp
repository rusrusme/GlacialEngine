//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include "core/Application.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <core/Config.hpp>
#include <GLFW/glfw3.h>
#include <input/CameraController.hpp>
#include <input/Input.hpp>
#include <render/Camera.hpp>
#include <render/Frame.hpp>

#if defined(_WIN32)
#include <windows.h>
// ReSharper disable once CppWrongIncludesOrder
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace engine {
    namespace {
        struct ApplicationContext {
            Renderer* renderer = nullptr;
            CameraController* camera_controller = nullptr;
        };

        void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
            const auto* context = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(window));
            if (context != nullptr && context->renderer != nullptr) {
                context->renderer->resize(width, height);

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
            setting::MONITOR_RESOLUTION_WIDTH,
            setting::MONITOR_RESOLUTION_HEIGHT,
            setting::FRAME_TITLE, nullptr, nullptr);
        if (!window) {
            std::cerr << "Engine error: application: failed to create window" << std::endl;
        }

        glfwMakeContextCurrent(window);

        // --------------------------------
        // Initialize frame
        // --------------------------------
        Frame::init(window);

        Renderer renderer;
        CameraController camera_controller;
        ApplicationContext context;
        context.renderer = &renderer;
        context.camera_controller = &camera_controller;

        if (!renderer.init(window)) {
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

        if (height > 0) {
            renderer.get_camera().set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        }
        renderer.get_camera().set_projection_params(45.0f, 0.1f, 10000.0f);

        // --------------------------------
        // Game logic
        // --------------------------------
        setting::config::load();
        init(renderer);

#if defined(_WIN32)
        timeBeginPeriod(1);
#endif

        double last_frame_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            const double frame_start_time = glfwGetTime();

            // ----------------------------------
            // Delta timer
            // ----------------------------------
            const double current_time = static_cast<float>(glfwGetTime());
            if (last_frame_time == 0.0f) {
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

            if (Input::is_key_pressed_once(window, GLFW_KEY_F3)) {
                renderer.toggle_debug_panel();
            }

            update(renderer, window, camera_controller, delta_time);

            // ----------------------------------
            // Render pass
            // ----------------------------------
            renderer.begin_frame();
            renderer.draw();
            render(renderer);

            Frame::end_frame();

            renderer.end_frame();

            glfwSwapBuffers(window);
            glfwPollEvents();

            // ----------------------------------
            // Frame rate limiter
            // ----------------------------------
            const int target_fps = setting::get_target_fps();
            if (target_fps > 0 && target_fps < INT32_MAX) {
                const double target_frame_time = 1.0 / static_cast<double>(target_fps);
                while (glfwGetTime() - frame_start_time < target_frame_time) {
                    const double time_remaining = target_frame_time - (glfwGetTime() - frame_start_time);
                    if (time_remaining > 0.002) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            }
        }

#if defined(_WIN32)
        timeEndPeriod(1);
#endif

        shutdown(renderer, window);
        return 0;
    }

    void Application::shutdown(Renderer& renderer, GLFWwindow* window) {
        setting::config::save();

        Frame::shutdown();
        renderer.shutdown();
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
}
