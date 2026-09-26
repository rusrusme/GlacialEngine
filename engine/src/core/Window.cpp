//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <core/Window.hpp>

#include "core/Config.hpp"
#include "core/WindowAPI.hpp"
#include "event/KeyEvents.hpp"
#include "event/MouseEvents.hpp"

namespace engine {
    Window::Window() {
        data_.title = setting::FRAME_TITLE;
        data_.width = setting::video::WINDOW_WIDTH;
        data_.height = setting::video::WINDOW_HEIGHT;

        CORE_INFO("Creating window: '{0}' ({1}x{2})", data_.title, data_.width, data_.height);

        window_ = glfwCreateWindow(data_.width, data_.height, data_.title.c_str(), nullptr, nullptr);
        // window_handle_ = window::create_window(data_.width, data_.height, data_.title.c_str(), &data_);
        if (!window_) {
            CORE_CRITICAL("Could not create GLFW window!");
            return;
        }

        // Apply saved position & window mode
        if (setting::video::WINDOW_X >= 0 && setting::video::WINDOW_Y >= 0) {
            glfwSetWindowPos(window_, setting::video::WINDOW_X, setting::video::WINDOW_Y);
        }

        if (setting::video::IS_MAXIMIZED) {
            glfwMaximizeWindow(window_);
        }

        glfwMakeContextCurrent(window_);

        update_dimensions();
        setup_callbacks();
    }

    Window::~Window() {
        if (window_) {
            save_state();
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
    }

    void Window::set_event_callback(const EventCallbackFunc& callback) {
        data_.event_callback = callback;
    }

    void Window::set_vsync(const bool enabled) {
        setting::video::IS_VSYNC = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Window::set_title(const std::string& title) {
        data_.title = title;
        if (window_) {
            glfwSetWindowTitle(window_, data_.title.c_str());
        }
    }

    float Window::get_monitor_dpi_scale() {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (!monitor) return 1.0f;
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        return xscale;
    }

    void Window::save_state() {
        if (!window_) return;

        const bool is_iconified = glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0;
        const bool is_max = glfwGetWindowAttrib(window_, GLFW_MAXIMIZED) != 0;

        setting::video::IS_MAXIMIZED = is_max;

        if (!is_iconified && !is_max) {
            glfwGetWindowPos(window_, &setting::video::WINDOW_X, &setting::video::WINDOW_Y);
            glfwGetWindowSize(window_, &setting::video::WINDOW_WIDTH, &setting::video::WINDOW_HEIGHT);
        }
    }

    void Window::update_dimensions() {
        if (window_) {
            glfwGetFramebufferSize(window_, &data_.width, &data_.height);
        }
    }

    void Window::setup_callbacks() {
        if (!window_) return;

        glfwSetWindowUserPointer(window_, &data_);

        // ------------------------------------------------------------------------------
        // Window events
        // ------------------------------------------------------------------------------
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, const int width, const int height) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data && data->event_callback) {
                data->width = width;
                data->height = height;
                WindowResizedEvent event(width, height);
                data->event_callback(event);
            }
        });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data && data->event_callback) {
                WindowClosedEvent event;
                data->event_callback(event);
            }
        });

        // ------------------------------------------------------------------------------
        // Keyboard events
        // ------------------------------------------------------------------------------
        glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->event_callback) return;

            switch (action) {
                case GLFW_PRESS : {
                    KeyPressedEvent event(key, false);
                    data->event_callback(event);
                    break;
                }
                case GLFW_RELEASE : {
                    KeyReleasedEvent event(key);
                    data->event_callback(event);
                    break;
                }
                case GLFW_REPEAT : {
                    KeyPressedEvent event(key, true);
                    data->event_callback(event);
                    break;
                }
                default : break;
            }
        });

        glfwSetCharCallback(window_, [](GLFWwindow* window, unsigned int keycode) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->event_callback) return;
            KeyTypedEvent event(static_cast<int>(keycode));
            data->event_callback(event);
        });

        // ------------------------------------------------------------------------------
        // Mouse events
        // ------------------------------------------------------------------------------
        glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->event_callback) return;

            if (action == GLFW_PRESS) {
                MouseButtonPressedEvent event(button);
                data->event_callback(event);
            }
            else if (action == GLFW_RELEASE) {
                MouseButtonReleasedEvent event(button);
                data->event_callback(event);
            }
        });

        glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data || !data->event_callback) return;
            MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
            data->event_callback(event);
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
