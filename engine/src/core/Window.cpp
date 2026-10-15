//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <core/Window.hpp>
#include <utility>

#include "core/Config.hpp"
#include "core/WindowSystem.hpp"

namespace engine {
    Window::Window(std::string title) : data_{
        .title  = std::move(title),
        .width  = config::video::WINDOW_WIDTH,
        .height = config::video::WINDOW_HEIGHT
    } {
        CORE_INFO("Creating window: '{0}' ({1}x{2})", data_.title, data_.width, data_.height);

        window_handle_ = window::create(
            data_.width,
            data_.height,
            data_.title.c_str(),
            &data_
        );
        if (!window_handle_) {
            CORE_CRITICAL("Could not create window");
            return;
        }

        if (config::video::WINDOW_X >= 0 && config::video::WINDOW_Y >= 0) {
            window::position(window_handle_, config::video::WINDOW_X, config::video::WINDOW_Y);
        }
        if (config::video::IS_MAXIMIZED) {
            window::maximize(window_handle_);
        }

        window::make_context_current(window_handle_);

        update_dimensions();
    }

    Window::~Window() {
        if (window_handle_) {
            save_config();
            window::destroy(window_handle_);
            window_handle_ = nullptr;
        }
    }

    void Window::set_title(const std::string& title) {
        data_.title = title;
        if (window_handle_) {
            window::set_title(window_handle_, data_.title.c_str());
        }
    }

    void Window::set_event_callback(const EventCallbackFunc& callback) {
        data_.event_callback = callback;
    }

    void Window::set_vsync(const bool enabled) {
        config::video::IS_VSYNC = enabled;
        window::set_vsync(enabled);
    }

    float Window::monitor_dpi_scale() const {
        if (!window_handle_) return 1.0f;
        return window::monitor_dpi_scale();
    }

    void Window::save_config() const {
        if (!window_handle_) return;

        const bool is_minimized = window::minimized(window_handle_);
        const bool is_max = window::maximized(window_handle_);

        config::video::IS_MAXIMIZED = is_max;

        if (!is_minimized && !is_max) {
            window::position(window_handle_, &config::video::WINDOW_X, &config::video::WINDOW_Y);
            window::size(window_handle_, &config::video::WINDOW_WIDTH, &config::video::WINDOW_HEIGHT);
        }
    }

    void Window::update_dimensions() {
        if (window_handle_) {
            window::framebuffer_size(window_handle_, &data_.width, &data_.height);
        }
    }
}
