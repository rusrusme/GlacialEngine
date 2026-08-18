//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <functional>
#include <event/WindowEvents.hpp>

#include "GLFW/glfw3.h"

namespace engine {
    class Window {
    public:
        using EventCallbackFunc = std::function<void(Event &)>;

        explicit Window(GLFWwindow* window);

        void set_event_callback(const EventCallbackFunc& callback);

        void set_vsync(bool enabled);
        [[nodiscard]] bool is_vsync() const { return vsync_enabled_; }

        [[nodiscard]] float get_aspect_ratio() const { return data_.height > 0 ? static_cast<float>(data_.width) / static_cast<float>(data_.height) : 1.0f; }

        [[nodiscard]] uint32_t get_width() const { return data_.width; }

        [[nodiscard]] uint32_t get_height() const { return data_.height; }

        /** @brief Returns the raw window reference */
        [[nodiscard]] GLFWwindow* native() const { return window_; }

    private:
        struct WindowData {
            int width = 0;
            int height = 0;
            EventCallbackFunc event_callback;
        };

        void update_dimensions();

        void setup_callbacks();

        GLFWwindow* window_ = nullptr;
        bool vsync_enabled_ = false;
        WindowData data_;
    };
}
