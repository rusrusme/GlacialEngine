//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <core/Window.hpp>

namespace engine {
    Window::Window(GLFWwindow* window) : window_(window) {
        update_dimensions();
        setup_callbacks();
    }

    void Window::set_event_callback(const EventCallbackFunc& callback) {
        data_.event_callback = callback;
    }

    void Window::on_resize(const int width, const int height) {
        width_ = width;
        height_ = height;
    }

    void Window::set_vsync(const bool enabled) {
        vsync_enabled_ = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Window::update_dimensions() {
        if (window_) {
            glfwGetFramebufferSize(window_, &width_, &height_);
        }
    }

    void Window::setup_callbacks() {
        if (!window_) return;

        data_.width = width_;
        data_.height = height_;

        glfwSetWindowUserPointer(window_, &data_);

        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, const int width, const int height) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data && data->event_callback) {
                data->width = width;
                data->height = height;
                WindowResizedEvent event(width, height);
                data->event_callback(event);
            }
        });

        glfwSetScrollCallback(window_, [](GLFWwindow* window, const double x_offset, const double y_offset) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data && data->event_callback) {
                MouseScrolledEvent event(static_cast<float>(x_offset), static_cast<float>(y_offset));
                data->event_callback(event);
            }
        });
    }
}
