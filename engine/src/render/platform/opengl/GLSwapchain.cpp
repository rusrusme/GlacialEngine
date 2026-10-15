//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLSwapchain.hpp"

#include "core/Log.hpp"

namespace engine {
    GLSwapchain::GLSwapchain(const SwapchainSpec& spec)
        : spec_(spec), window_(static_cast<GLFWwindow*>(spec.window_handle)) {
        if (!window_) {
            CORE_ERROR("[GLSwapchain] Window handle provided is null");
        }

        set_vsync(spec_.vsync);
    }

    GLSwapchain::GLSwapchain(GLSwapchain&& other) noexcept
        : spec_(other.spec_),
          window_(other.window_),
          current_frame_index_(other.current_frame_index_) {
        other.window_ = nullptr;
    }

    GLSwapchain& GLSwapchain::operator=(GLSwapchain&& other) noexcept {
        if (this != &other) {
            spec_ = other.spec_;
            window_ = other.window_;
            current_frame_index_ = other.current_frame_index_;
            other.window_ = nullptr;
        }
        return *this;
    }

    void GLSwapchain::present() {
        if (window_) {
            glfwSwapBuffers(window_);
        }
        current_frame_index_ = (current_frame_index_ + 1) % spec_.max_frames_in_flight;
    }

    void GLSwapchain::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        spec_.width = width;
        spec_.height = height;
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void GLSwapchain::set_vsync(bool enabled) {
        spec_.vsync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }
}
