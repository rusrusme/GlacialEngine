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
#include <event/MouseScrolledEvent.hpp>
#include <event/WindowResizedEvent.hpp>
#include <GLFW/glfw3.h>
#include <render/Frame.hpp>

namespace engine {
    namespace {
        void framebuffer_size_callback(GLFWwindow* window_, const int width, const int height) {
            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window_));
            if (app) {
                WindowResizedEvent event(width, height);
                app->on_event(event);
            }
        }

        void scroll_callback(GLFWwindow* window, const double x_offset, const double y_offset) {
            auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (app) {
                MouseScrolledEvent event(static_cast<float>(x_offset), static_cast<float>(y_offset));
                app->on_event(event);
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

        GLFWwindow* raw_window = glfwCreateWindow(
            setting::video::MONITOR_RESOLUTION_WIDTH,
            setting::video::MONITOR_RESOLUTION_HEIGHT,
            setting::FRAME_TITLE, nullptr, nullptr);
        if (!raw_window) {
            std::cerr << "Engine error: application: failed to create window_" << std::endl;
            glfwTerminate();
            return 1;
        }

        glfwMakeContextCurrent(raw_window);

        // Initialize window
        window_ = std::make_unique<Window>(raw_window);

        // ----------------------------------
        // Initialize core systems
        // ----------------------------------
        Log::init();
        Frame::init(window_->native());

        if (!renderer_.on_init(window_->native())) {
            std::cerr << "Engine error: application: failed to initialize renderer" << std::endl;
            glfwDestroyWindow(window_->native());
            glfwTerminate();
            return 1;
        }

        glfwSetWindowUserPointer(window_->native(), this);
        glfwSetFramebufferSizeCallback(window_->native(), framebuffer_size_callback);
        // Todo: move to separate on_event() method or EventDispatcher class
        glfwSetScrollCallback(window_->native(), scroll_callback);

        for (const auto& layer : layers_) {
            layer->on_attach(renderer_);
        }

        // ----------------------------------
        // Run core loop
        // ----------------------------------
        setting::configuration::load();

        while (!glfwWindowShouldClose(window_->native())) {
            const float delta_time = Frame::begin_frame();

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
                layer->on_render(renderer_, delta_time);
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

        shutdown(renderer_);
        return 0;
    }

    void Application::shutdown(Renderer& renderer) {
        setting::configuration::save();

        Frame::shutdown();
        renderer_.on_shutdown();
        if (window_ != nullptr) {
            glfwDestroyWindow(window_->native());
        }
        glfwTerminate();
    }

    void Application::on_event(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizedEvent>([this](WindowResizedEvent& e) {
            window_->on_resize(e.width(), e.height());
            renderer_.on_resize(e.width(), e.height());
            return false; // Weiterleiten an Layer!
        });

        // From top to bottom
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            if (event.handled) {
                break; // Event consumed
            }
            (*it)->on_event(event);
        }
    }

    void Application::push_layer(std::unique_ptr<Layer> layer) {
        layers_.push_layer(std::move(layer));
    }

    void Application::push_overlay(std::unique_ptr<Layer> overlay) {
        layers_.push_overlay(std::move(overlay));
    }

    Application* Application::instance_ = nullptr;
}
