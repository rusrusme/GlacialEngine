//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once
#include "GLFW/glfw3.h"

namespace engine {
    class Window {
    public:
        explicit Window(GLFWwindow* window) : window_(window) {
            update_dimensions();
        }

        void on_resize(const int width, const int height) {
            width_ = width;
            height_ = height;
        }

        void set_vsync(bool enabled) {
            vsync_enabled_ = enabled;
            glfwSwapInterval(enabled ? 1 : 0);
        }

        [[nodiscard]] bool is_vsync() const { return vsync_enabled_; }

        [[nodiscard]] float get_aspect_ratio() const {
            return height_ > 0 ? static_cast<float>(width_) / static_cast<float>(height_) : 1.0f;
        }

        [[nodiscard]] uint32_t get_width() const { return width_; }

        [[nodiscard]] uint32_t get_height() const { return height_; }

        /** @brief Returns the raw window reference */
        [[nodiscard]] GLFWwindow* native() const { return window_; }

    private:
        void update_dimensions() {
            if (window_) {
                glfwGetFramebufferSize(window_, &width_, &height_);
            }
        }

        GLFWwindow* window_ = nullptr;
        int width_ = 0;
        int height_ = 0;
        bool vsync_enabled_ = false;
    };
}
