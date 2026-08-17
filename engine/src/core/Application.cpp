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
#include <render/Frame.hpp>

namespace engine {
    namespace {
        struct ApplicationContext {
            Renderer* renderer = nullptr;
            CameraController* camera_controller = nullptr;
        };

        void framebuffer_size_callback(GLFWwindow* window_, const int width, const int height) {
            const auto* context = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(window_));
            if (context != nullptr && context->renderer != nullptr) {
                context->renderer->on_resize(width, height);

                if (height > 0) {
                    const float aspect = static_cast<float>(width) / static_cast<float>(height);
                    // context->renderer->get_camera().set_aspect_ratio(aspect);
                }
            }
        }

        void scroll_callback(GLFWwindow* window_, double /*x_offset*/, const double y_offset) {
            if (Input::is_mouse_captured()) {
                return;
            }
            const auto* context = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(window_));
            if (context != nullptr && context->renderer != nullptr && context->camera_controller != nullptr) {
                // CameraController::on_scroll(context->renderer->get_camera(), y_offset);
            }
        }
    }

    Application::Application() {
        instance_ = this;
    }

    int Application::run() {
        if (!glfwInit()) {
            std::cerr << "Engine error: application: failed to initialize GLFW" << std::endl;
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(
            setting::video::MONITOR_RESOLUTION_WIDTH,
            setting::video::MONITOR_RESOLUTION_HEIGHT,
            setting::FRAME_TITLE, nullptr, nullptr);
        if (!window_) {
            std::cerr << "Engine error: application: failed to create window_" << std::endl;
            glfwTerminate();
            return 1;
        }

        glfwMakeContextCurrent(window_);

        // ----------------------------------
        // Initialize core systems
        // ----------------------------------
        Log::init();
        Frame::init(window_);

        Renderer renderer;
        CameraController camera_controller;
        ApplicationContext context;
        context.renderer = &renderer;
        context.camera_controller = &camera_controller;

        if (!renderer.on_init(window_)) {
            std::cerr << "Engine error: application: failed to initialize renderer" << std::endl;
            glfwDestroyWindow(window_);
            glfwTerminate();
            return 1;
        }

        glfwSetWindowUserPointer(window_, &context);
        glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
        glfwSetScrollCallback(window_, scroll_callback);

        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);

        for (const auto& layer : layers_) {
            layer->on_attach(renderer);
        }
        // if (height > 0) {
            // renderer.get_camera().set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
        // }
        // renderer.get_camera().set_projection_params(45.0f, 0.1f, 10000.0f);

        // ----------------------------------
        // Run core loop
        // ----------------------------------
        setting::configuration::load();

        while (!glfwWindowShouldClose(window_)) {
            const float delta_time = Frame::begin_frame();

            // Todo: remove this
            // if (!Input::is_mouse_captured()) {
            //     // camera_controller.update(window_, renderer.get_camera(), delta_time);
            // }
            // else {
            //     camera_controller.reset_mouse_state();
            // }

            // ----------------------------------
            // Update pass
            // ----------------------------------
            for (const auto& layer : layers_) {
                layer->on_update(delta_time);
            }

            // ----------------------------------
            // Render pass
            // ----------------------------------
            for (const auto& layer : layers_) {
                layer->on_render(renderer, delta_time);
            }
            #ifdef ENABLE_IMGUI
            for (const auto& layer : layers_) {
                layer->on_imGuiRender(delta_time);
            }
            #endif
            // ----------------------------------

            Frame::end_frame();
        }
        // In reverse
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_detach();
        }

        shutdown(renderer);
        return 0;
    }

    void Application::shutdown(Renderer& renderer) const {
        setting::configuration::save();

        Frame::shutdown();
        renderer.on_shutdown();
        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
        }
        glfwTerminate();
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layers_.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        layers_.push_overlay(std::move(overlay));
    }

    Application* Application::instance_ = nullptr;
}
