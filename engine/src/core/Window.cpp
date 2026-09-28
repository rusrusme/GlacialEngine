//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <core/Window.hpp>
#include <utility>

#include "core/Config.hpp"
#include "core/WindowAPI.hpp"

namespace engine {
    Window::Window(std::string title) : data_{
        .title  = std::move(title),
        .width  = setting::video::WINDOW_WIDTH,
        .height = setting::video::WINDOW_HEIGHT
    } {
        CORE_INFO("Creating window: '{0}' ({1}x{2})", data_.title, data_.width, data_.height);

        window_handle_ = window::create_window(
            data_.width,
            data_.height,
            data_.title.c_str(),
            &data_
        );
        if (!window_handle_) {
            CORE_CRITICAL("Could not create window");
            return;
        }

        // Apply saved position & window mode
        if (setting::video::WINDOW_X >= 0 && setting::video::WINDOW_Y >= 0) {
            window::set_position(window_handle_, setting::video::WINDOW_X, setting::video::WINDOW_Y);
        }

        if (setting::video::IS_MAXIMIZED) {
            window::maximize_window(window_handle_);
        }

        window::make_context_current(window_handle_);

        update_dimensions();
    }

    Window::~Window() {
        if (window_handle_) {
            save_state();
            window::destroy_window(window_handle_);
            window_handle_ = nullptr;
        }
    }

    void Window::set_event_callback(const EventCallbackFunc& callback) {
        data_.event_callback = callback;
    }

    void Window::set_vsync(const bool enabled) {
        setting::video::IS_VSYNC = enabled;
        window::set_vsync(enabled);
    }

    void Window::set_title(const std::string& title) {
        data_.title = title;
        if (window_handle_) {
            window::set_title(window_handle_, data_.title.c_str());
        }
    }

    float Window::get_monitor_dpi_scale() const {
        if (!window_handle_) return 1.0f;
        return window::get_monitor_dpi_scale();
    }

    void Window::save_state() const {
        if (!window_handle_) return;

        const bool is_minimized = window::is_minimized(window_handle_);
        const bool is_max = window::is_maximized(window_handle_);

        setting::video::IS_MAXIMIZED = is_max;

        if (!is_minimized && !is_max) {
            window::get_position(window_handle_, &setting::video::WINDOW_X, &setting::video::WINDOW_Y);
            window::get_size(window_handle_, &setting::video::WINDOW_WIDTH, &setting::video::WINDOW_HEIGHT);
        }
    }

    void Window::update_dimensions() {
        if (window_handle_) {
            window::get_framebuffer_size(window_handle_, &data_.width, &data_.height);
        }
    }
}
