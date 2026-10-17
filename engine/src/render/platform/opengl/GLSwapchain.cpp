//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "render/platform/opengl/GLSwapchain.hpp"

#include "core/Log.hpp"
#include "core/WindowSystem.hpp"
#include "render/platform/opengl/GLFramebuffer.hpp"

namespace engine {
    GLSwapchain::GLSwapchain(const SwapchainSpec& spec)
        : spec_(spec),
          window_(spec.window_handle) {
        if (!window_) {
            CORE_ERROR("[GLSwapchain] Window handle provided is null");
        }

        FramebufferSpecification fbo_spec{};
        fbo_spec.width = spec_.width;
        fbo_spec.height = spec_.height;
        fbo_spec.swapchain_target = true;
        backbuffer_ = std::make_unique<GLFramebuffer>(fbo_spec);

        set_vsync(spec_.vsync);
    }

    GLSwapchain::GLSwapchain(GLSwapchain&& other) noexcept
        : spec_(other.spec_),
          window_(std::exchange(other.window_, nullptr)),
          backbuffer_(std::move(other.backbuffer_)) {}

    GLSwapchain& GLSwapchain::operator=(GLSwapchain&& other) noexcept {
        if (this != &other) {
            spec_ = other.spec_;
            window_ = std::exchange(other.window_, nullptr);
            backbuffer_ = std::move(other.backbuffer_);
        }
        return *this;
    }

    void GLSwapchain::present() {
        if (window_) {
            window::swap_buffers(window_);
        }
    }

    void GLSwapchain::resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            return;
        }
        spec_.width = width;
        spec_.height = height;
        if (backbuffer_) {
            backbuffer_->resize(width, height);
        }
    }

    void GLSwapchain::set_vsync(bool enabled) {
        spec_.vsync = enabled;
        window::set_vsync(enabled);
    }
}
